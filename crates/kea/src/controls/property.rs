use bevy::{
    ecs::system::IntoObserverSystem,
    prelude::*,
};
use crate::{
    controls::{
        label::KeaLabel,
        text::KeaTextInput,
    },
    overrides::KeaNodeOverrides,
};

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct KeaProperty {
    _private: (),
}

impl KeaProperty {
    pub fn bundle<E: Event, B: Bundle, M>(
        label: &str,
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
                KeaTextInput::bundle_with_callback(callback),
                KeaNodeOverrides {
                    flex_grow: Some(1.0),
                    ..default()
                }
            ),
        ]
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
