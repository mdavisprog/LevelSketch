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
    observers::KeaObservers,
};
use super::systems::{
    on_text_confirm,
    on_unfocus,
};

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct KeaPropertyText {
    pub(super) value: String,
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
            value: format.convert(text),
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
                KeaObservers::<Self>::new(vec![
                    Observer::new(on_unfocus),
                ]),
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            column_gap: Val::Px(6.0),
            align_items: AlignItems::Center,
            ..default()
        }
    }
}
