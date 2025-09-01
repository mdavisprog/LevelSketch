use bevy::prelude::*;
use crate::{
    constants,
    controls::button::KeaButton,
    observers::KeaObservers,
    overrides::KeaNodeOverrides,
    style,
};
use super::systems::{
    on_close,
    on_header_drag,
};

#[derive(Debug, Clone, Copy)]
pub enum KeaPanelCloseBehavior {
    Close,
    Trigger,
}

///
/// KeaPanelOptions
///
pub struct KeaPanelOptions {
    pub title: String,
    pub position: Vec2,
    pub size: Vec2,
    pub close_behavior: KeaPanelCloseBehavior,
}

impl Default for KeaPanelOptions {
    fn default() -> Self {
        Self {
            title: format!(""),
            position: Vec2::ZERO,
            size: Vec2::ZERO,
            close_behavior: KeaPanelCloseBehavior::Close,
        }
    }
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
    pub(super) close_behavior: KeaPanelCloseBehavior,
}

impl KeaPanel {
    pub fn bundle(
        options: KeaPanelOptions,
        contents: impl Bundle,
    ) -> impl Bundle {(
        Self {
            close_behavior: options.close_behavior,
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
            row_gap: Val::Px(4.0),
            overflow: Overflow::clip(),
            ..default()
        },
        children![
            (
                KeaPanelHeader::bundle(&options.title),
            ),
            (
                Node {
                    width: Val::Percent(100.0),
                    height: Val::Percent(100.0),
                    overflow: Overflow::scroll(),
                    align_items: AlignItems::Start,
                    ..default()
                },
                children![(
                    contents,
                )],
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
        KeaObservers::<Self>::new(vec![
            Observer::new(on_header_drag),
        ]),
        children![
            (
                KeaPanelTitle::bundle(title),
            ),
            (
                KeaButton::image_bundle(
                    &format!(
                        "kea://icons/close.svg#image{size}x{size}",
                        size = style::properties::PANEL_HEADER_FONT_SIZE
                    ),
                    on_close,
                ),
                KeaNodeOverrides {
                    padding: Some(UiRect::ZERO),
                    ..default()
                }
            ),
        ]
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            align_items: AlignItems::Center,
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Node = Self::node(),
    TextFont = TextFont::from_font_size(style::properties::PANEL_HEADER_FONT_SIZE),
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
