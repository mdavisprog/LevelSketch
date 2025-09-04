use bevy::{
    ecs::system::IntoObserverSystem,
    prelude::*,
};
use crate::{
    controls::{
        label::KeaLabel,
        text::{
            KeaTextInput,
            KeaTextInputFormat,
        },
    },
    overrides::KeaNodeOverrides,
};
use super::systems::on_text_confirm;

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct KeaPropertyText {
    _private: (),
}

impl KeaPropertyText {
    pub fn bundle(label: &str, text: &str) -> impl Bundle {
        Self::bundle_with_format_and_callback(
            label,
            text,
            KeaTextInputFormat::Default,
            on_text_confirm,
        )
    }

    pub(super) fn bundle_with_format_and_callback<E: Event, B: Bundle, M>(
        label: &str,
        text: &str,
        format: KeaTextInputFormat,
        callback: impl IntoObserverSystem<E, B, M>,
    ) -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            (
                KeaLabel::bundle(label),
            ),
            (
                KeaTextInput::bundle_with_callback_and_text(
                    &format.convert(text),
                    format,
                    callback,
                ),
                KeaNodeOverrides {
                    flex_grow: Some(1.0),
                    ..default()
                },
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            column_gap: Val::Px(6.0),
            align_items: AlignItems::Center,
            align_content: AlignContent::Stretch,
            ..default()
        }
    }
}
