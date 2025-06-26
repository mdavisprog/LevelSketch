use bevy::prelude::*;
use crate::style;

#[derive(Component)]
#[require(
    TextFont = TextFont::from_font_size(style::properties::FONT_SIZE),
)]
pub struct KeaLabel {
    _private: (),
}

impl KeaLabel {
    pub fn bundle(text: &str) -> impl Bundle {(
        Self {
            _private: (),
        },
        Text::new(text),
    )}
}
