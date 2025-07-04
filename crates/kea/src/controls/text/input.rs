use bevy::{
    ecs::system::IntoObserverSystem,
    prelude::*,
};
use crate::{
    observers::KeaObservers,
    style,
};
use super::document::Document;

#[derive(Default)]
pub struct KeaTextInputFormatNumber {
    pub precision: usize,
}

pub enum KeaTextInputFormat {
    Default,
    Numbers(KeaTextInputFormatNumber),
}

impl KeaTextInputFormat {
    pub fn convert(&self, text: &str) -> String {
        match self {
            Self::Default => text.to_string(),
            Self::Numbers(format) => {
                let value = if let Ok(value) = text.parse::<f32>() {
                    value
                } else {
                    0.0
                };

                format!("{:.prec$}", value, prec = format.precision)
            }
        }
    }
}

#[derive(Component)]
#[require(
    Node = Self::node(),
    BorderRadius = style::properties::BORDER_RADIUS,
    BackgroundColor = style::colors::BUTTON_BACKGROUND,
)]
pub struct KeaTextInput {
    pub(super) format: KeaTextInputFormat,
}

impl KeaTextInput {
    pub fn bundle() -> impl Bundle {
        Self::bundle_with_text("", KeaTextInputFormat::Default)
    }

    pub fn bundle_with_text(text: &str, format: KeaTextInputFormat) -> impl Bundle {(
        Self {
            format,
        },
        children![
            Document::bundle(text),
        ]
    )}

    pub fn bundle_with_callback<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>
    ) -> impl Bundle {
        Self::bundle_with_callback_and_text(
            "",
            KeaTextInputFormat::Default,
            callback
        )
    }

    pub fn bundle_with_callback_and_text<E: Event, B: Bundle, M>(
        text: &str,
        format: KeaTextInputFormat,
        callback: impl IntoObserverSystem<E, B, M>,
    ) -> impl Bundle {(
        Self {
            format,
        },
        children![
            (
                Document::bundle(text),
            ),
            (
                KeaObservers::new_observe_parent(vec![
                    Observer::new(callback),
                ]),
            )
        ],
    )}

    fn node() -> Node {
        Node {
            overflow: Overflow::clip(),
            width: Val::Px(75.0),
            height: Val::Px(style::properties::FONT_SIZE + 6.0),
            padding: UiRect::horizontal(Val::Px(6.0)),
            justify_content: JustifyContent::Center,
            ..default()
        }
    }
}
