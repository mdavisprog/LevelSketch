use bevy::prelude::*;
use crate::style;

#[derive(Component)]
pub struct KeaSeparator {
    _private: (),
}

impl KeaSeparator {
    pub fn horizontal() -> impl Bundle {(
        Self {
            _private: (),
        },
        Node {
            width: Val::Percent(100.0),
            height: Val::Px(style::properties::SEPARATOR_GAP),
            align_items: AlignItems::Center,
            padding: UiRect::horizontal(Val::Px(style::properties::SEPARATOR_PADDING)),
            ..default()
        },
        children![(
            Node {
                width: Val::Percent(100.0),
                height: Val::Px(style::properties::SEPARATOR_SIZE),
                ..default()
            },
            BackgroundColor(style::colors::SEPARATOR),
        )]
    )}

    pub fn vertical() -> impl Bundle {(
        Self {
            _private: (),
        },
        Node {
            width: Val::Px(style::properties::SEPARATOR_GAP),
            height: Val::Percent(100.0),
            justify_content: JustifyContent::Center,
            padding: UiRect::vertical(Val::Px(style::properties::SEPARATOR_PADDING)),
            ..default()
        },
        children![(
            Node {
                width: Val::Px(style::properties::SEPARATOR_SIZE),
                height: Val::Percent(100.0),
                ..default()
            },
            BackgroundColor(style::colors::SEPARATOR),
        )]
    )}
}
