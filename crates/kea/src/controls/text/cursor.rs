use bevy::prelude::*;
use crate::style;

#[derive(Component, Default)]
#[require(
    Node = Self::node(),
    BackgroundColor(style::colors::CURSOR),
    Visibility = Visibility::Hidden,
)]
pub(super) struct Cursor {
    pub(super) index: usize,
}

impl Cursor {
    fn node() -> Node {
        Node {
            width: Val::Px(2.0),
            height: Val::Px(style::properties::FONT_SIZE),
            position_type: PositionType::Absolute,
            ..default()
        }
    }
}
