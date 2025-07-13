use bevy::prelude::*;
use crate::{
    animation::{
        KeaAnimation,
        KeaAnimationClip,
        KeaAnimationComplete,
    },
    observers::KeaObservers,
    style,
};
use super::{
    image::KeaImageNode,
    separator::KeaSeparator,
};

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum KeaExpanderState {
    Expanded,
    Collapsed,
}

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct KeaExpander {
    state: KeaExpanderState,
    expanded_height: f32,
}

impl KeaExpander {
    pub fn bundle(label: &str, contents: impl Bundle) -> impl Bundle {(
        Self {
            state: KeaExpanderState::Expanded,
            expanded_height: 0.0,
        },
        children![
            (
                Header::bundle(label),
            ),
            (
                KeaSeparator::horizontal(),
            ),
            (
                // Clips the entities with contents when being collapsed.
                Node {
                    width: Val::Percent(100.0),
                    height: Val::Percent(100.0),
                    overflow: Overflow::clip(),
                    ..default()
                },
                children![(
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
                )],
            ),
        ]
    )}

    pub fn is_expanded(&self) -> bool {
        self.state == KeaExpanderState::Expanded
    }

    pub fn is_collapsed(&self) -> bool {
        self.state == KeaExpanderState::Collapsed
    }

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
#[require(
    KeaObservers = Self::observers(),
)]
struct Contents;

impl Contents {
    fn observers() -> KeaObservers {
        KeaObservers::new(vec![
            Observer::new(on_contents_anim_complete),
        ])
    }
}

fn on_click_header(
    trigger: Trigger<Pointer<Click>>,
    children: Query<&ChildOf>,
    parents: Query<&Children>,
    contents: Query<&Contents>,
    images: Query<&ImageNode>,
    computed_nodes: Query<&ComputedNode>,
    mut expanders: Query<&mut KeaExpander>,
    mut nodes: Query<&mut Node>,
    mut commands: Commands,
) {
    let Ok(child) = children.get(trigger.target()) else {
        return;
    };

    // The parent should be the expander component.
    let Ok(mut expander) = expanders.get_mut(child.parent()) else {
        return;
    };

    // Find the entity that has the Contents component.
    let mut contents_entity = Entity::PLACEHOLDER;
    for child in parents.iter_descendants(child.parent()) {
        if contents.contains(child) {
            contents_entity = child;
            break;
        }
    }

    let Ok(computed_node) = computed_nodes.get(contents_entity) else {
        return;
    };

    let node_height = computed_node.content_size.y;

    // Update the expander state.
    expander.state = match expander.state {
        KeaExpanderState::Expanded => {
            expander.expanded_height = node_height;
            KeaExpanderState::Collapsed
        },
        KeaExpanderState::Collapsed => KeaExpanderState::Expanded,
    };

    // If the expander is being collapsed, override the height value to be Val::Px so that
    // there is no visual artifact caused if starting value is Val::Percent.
    if let Ok(mut node) = nodes.get_mut(contents_entity) {
        if expander.state == KeaExpanderState::Collapsed {
            node.height = Val::Px(node_height);
        }
    };

    let (top, height) = match expander.state {
        KeaExpanderState::Expanded => (Val::Px(0.0), Val::Px(expander.expanded_height)),
        KeaExpanderState::Collapsed => (Val::Px(-node_height), Val::Px(0.0)),
    };

    // Animate the node to move up, shrink in height, and clip.
    commands
        .entity(contents_entity)
        .insert(
            KeaAnimation::new_with_clips(vec![
                KeaAnimationClip::new::<Node>("top", top, 0.5),
                KeaAnimationClip::new::<Node>("height", height, 0.5),
            ]),
        );

    // Update the rotation of the expander icon.
    for child in parents.iter_descendants(trigger.target()) {
        if images.contains(child) {
            let rotation = match expander.state {
                KeaExpanderState::Expanded => Quat::from_rotation_z(0.0),
                KeaExpanderState::Collapsed => Quat::from_rotation_z(180_f32.to_radians()),
            };

            commands
                .entity(child)
                .insert(KeaAnimation::new_with_clips(vec![
                    KeaAnimationClip::new::<Transform>("rotation", rotation, 0.5),
                ]));

            break;
        }
    }
}

fn on_contents_anim_complete(
    trigger: Trigger<KeaAnimationComplete>,
    expanders: Query<&KeaExpander>,
    children: Query<&ChildOf>,
    mut nodes: Query<&mut Node>,
) {
    let event = trigger.event();

    // The height value when fully expanded should be a Val::Percent(100.0). Unfortunately,
    // animating Val::Percent does not impact the computed node size so the height needs to
    // set manually. When the header is fully expanded and the animation completes, this
    // will restore the 'height' property to be Val::Percent(100.0).
    if event.field == "height" {
        let Ok(mut node) = nodes.get_mut(trigger.target()) else {
            return;
        };

        // Find the expander component entity and get the expanded state.
        for parent in children.iter_ancestors(trigger.target()) {
            let Ok(expander) = expanders.get(parent) else {
                continue;
            };

            if expander.state == KeaExpanderState::Expanded {
                node.height = Val::Percent(100.0);
            }

            break;
        }
    }
}
