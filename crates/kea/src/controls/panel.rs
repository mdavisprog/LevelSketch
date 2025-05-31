use bevy::prelude::*;
use crate::{
    constants,
    observers::KeaObservers,
    overrides::NodeOverrides,
    style,
};
use super::{
    anchors::KeaAnchors,
    sizer::KeaSizer,
    button::{
        KeaButton,
        KeaButtonClick,
    },
};

///
/// KeaPanelOptions
///
pub struct KeaPanelOptions {
    pub title: String,
    pub position: Vec2,
    pub size: Vec2,
}

///
/// KeaPanel
///
/// Control that hosts a given bundle of nodes within a container
/// and is draggable and resizable.
///
#[derive(Component)]
#[require(
    BackgroundColor(style::colors::BACKGROUND),
    ZIndex(constants::BASE_Z_INDEX),
)]
pub struct KeaPanel {
    _private: (),
}

impl KeaPanel {
    pub fn bundle(
        options: KeaPanelOptions,
        contents: impl Bundle,
    ) -> impl Bundle {(
        Self {
            _private: (),
        },
        Node {
            flex_direction: FlexDirection::Column,
            left: Val::Px(options.position.x),
            top: Val::Px(options.position.y),
            width: Val::Px(options.size.x),
            height: Val::Px(options.size.y),
            min_width: Val::Px(20.0),
            min_height: Val::Px(20.0),
            padding: UiRect::all(Val::Px(style::properties::PADDING)),
            ..default()
        },
        children![
            (
                KeaPanelHeader::bundle(&options.title),
            ),
            (
                contents,
            ),
            (
                KeaSizer::bundle(KeaAnchors::all()),
            ),
        ]
    )}
}

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
struct KeaPanelHeader;

impl KeaPanelHeader {
    fn bundle(title: &str) -> impl Bundle {(
        Self,
        KeaObservers::new(vec![
            Observer::new(on_header_drag),
        ]),
        children![
            (
                KeaPanelTitle::bundle(title),
            ),
            (
                KeaButton::image_bundle(on_close, "kea://icons/close.svg#image12x12"),
                NodeOverrides {
                    align_self: Some(AlignSelf::End),
                    padding: Some(UiRect::ZERO),
                    ..default()
                }
            ),
        ]
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Node = Self::node(),
    TextFont = TextFont::from_font_size(12.0),
    TextLayout = TextLayout::new_with_justify(JustifyText::Center),
    Pickable = Pickable::IGNORE,
)]
struct KeaPanelTitle;

impl KeaPanelTitle {
    fn bundle(title: &str) -> impl Bundle {(
        Self,
        Text::new(title),
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            align_self: AlignSelf::Center,
            ..default()
        }
    }
}

fn on_close(
    trigger: Trigger<KeaButtonClick>,
    child: Query<&ChildOf>,
    mut commands: Commands,
) {
    let root = child.root_ancestor(trigger.target());

    commands
        .entity(root)
        .try_despawn();
}

fn on_header_drag(
    trigger: Trigger<Pointer<Drag>>,
    children: Query<&ChildOf>,
    mut panels: Query<&mut Node, With<KeaPanel>>,
) {
    let Ok(child) = children.get(trigger.target) else {
        return;
    };

    let Ok(mut panel) = panels.get_mut(child.parent()) else {
        return;
    };

    let left = match panel.left {
        Val::Px(value) => value,
        _ => 0.0,
    };

    let top = match panel.top {
        Val::Px(value) => value,
        _ => 0.0,
    };

    let delta = trigger.delta;
    panel.left = Val::Px(left + delta.x);
    panel.top = Val::Px(top + delta.y);
}
