use bevy::prelude::*;
use crate::style;
use super::cursor::Cursor;

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub(super) struct Document {
    _private: (),
}

impl Document {
    pub fn bundle(text: &str) -> impl Bundle {
        Self::bundle_text_color(text, Color::WHITE)
    }

    pub fn bundle_text_color(text: &str, color: Color) -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            (
                DocumentContents::bundle(text),
                TextColor(color),
            ),
            (
                Cursor::default(),
            ),
        ]
    )}

    fn node() -> Node {
        Node {
            justify_content: JustifyContent::Center,
            align_items: AlignItems::Center,
            // Make the position absolute to prevent resizing the parent node.
            position_type: PositionType::Absolute,
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Text,
    TextFont = TextFont::from_font_size(style::properties::FONT_SIZE),
    TextLayout = TextLayout::new_with_linebreak(LineBreak::NoWrap),
    Pickable = Pickable::IGNORE,
)]
pub(super) struct DocumentContents {
    _private: (),
}

impl DocumentContents {
    fn bundle(text: &str) -> impl Bundle {(
        Self {
            _private: (),
        },
        Text::new(text),
    )}
}
