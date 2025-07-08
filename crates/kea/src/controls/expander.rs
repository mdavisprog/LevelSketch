use bevy::prelude::*;
use crate::{
    observers::KeaObservers,
    style,
};
use super::{
    image::KeaImageNode,
    separator::KeaSeparator,
};

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct KeaExpander {
    _private: (),
}

impl KeaExpander {
    pub fn bundle(label: &str, contents: impl Bundle) -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            (
                Header::bundle(label),
            ),
            (
                KeaSeparator::horizontal(),
            ),
            (
                Node {
                    width: Val::Percent(100.0),
                    height: Val::Percent(100.0),
                    overflow: Overflow::scroll(),
                    align_items: AlignItems::Start,
                    ..default()
                },
                Contents,
                children![(
                    contents,
                )],
            ),
        ]
    )}

    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(style::properties::ROW_GAP),
            width: Val::Percent(100.0),
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
struct Header;

impl Header {
    fn bundle(label: &str) -> impl Bundle {(
        Self,
        children![
            (
                Text::new(label),
                TextFont::from_font_size(style::properties::PANEL_HEADER_FONT_SIZE),
                TextLayout::new_with_justify(JustifyText::Center),
                Node {
                    width: Val::Percent(100.0),
                    ..default()
                },
                Pickable::IGNORE,
            ),
            (
                KeaImageNode(format!(
                    "kea://icons/expander.svg#image{size}x{size}",
                    size = style::properties::PANEL_HEADER_FONT_SIZE)
                ),
                Node {
                    align_self: AlignSelf::End,
                    ..default()
                },
                Pickable::IGNORE,
            ),
            (
                KeaObservers::new_observe_parent(vec![
                    Observer::new(on_click_header)
                ]),
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            align_items: AlignItems::Center,
            padding: UiRect::vertical(Val::Px(style::properties::PADDING)),
            ..default()
        }
    }
}

#[derive(Component)]
struct Contents;

fn on_click_header(
    trigger: Trigger<Pointer<Click>>,
    children: Query<&ChildOf>,
    parents: Query<&Children>,
    contents: Query<&Contents>,
    images: Query<&ImageNode>,
    mut transforms: Query<&mut Transform>,
    mut nodes: Query<(&mut Node, &mut Visibility)>,
) {
    let Ok(child) = children.get(trigger.target()) else {
        return;
    };

    let mut contents_entity = Entity::PLACEHOLDER;
    for child in parents.iter_descendants(child.parent()) {
        if contents.contains(child) {
            contents_entity = child;
            break;
        }
    }

    if contents_entity == Entity::PLACEHOLDER {
        return;
    }

    let Ok((mut node, mut visibility)) = nodes.get_mut(contents_entity) else {
        return;
    };

    node.display = match node.display {
        Display::Flex => Display::None,
        _ => Display::Flex,
    };

    *visibility = match node.display {
        Display::Flex => Visibility::Visible,
        _ => Visibility::Hidden,
    };

    for child in parents.iter_descendants(trigger.target()) {
        if images.contains(child) {
            let Ok(mut transform) = transforms.get_mut(child) else {
                continue;
            };

            transform.rotation = match *visibility {
                Visibility::Visible => Quat::from_rotation_z(0.0),
                _ => Quat::from_rotation_z(180_f32.to_radians()),
            };

            break;
        }
    }
}
