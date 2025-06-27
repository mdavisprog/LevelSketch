use bevy::{
    ecs::system::{
        IntoObserverSystem,
        SystemState,
    },
    prelude::*,
};
use crate::{
    controls::{
        label::KeaLabel,
        text::{
            KeaTextInput,
            KeaTextInputCommands,
        },
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
    ) -> impl Bundle {
        Self::bundle_with_text(label, "", callback)
    }

    pub fn bundle_with_text<E: Event, B: Bundle, M>(
        label: &str,
        text: &str,
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
                KeaTextInput::bundle_with_callback_and_text(callback, text),
                KeaNodeOverrides {
                    flex_grow: Some(1.0),
                    ..default()
                },
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

pub trait KeaPropertyCommandsExt {
    fn kea_property_set_value(&mut self, property: Entity, value: String) -> &mut Self;
}

impl<'w, 's> KeaPropertyCommandsExt for Commands<'w, 's> {
    fn kea_property_set_value(&mut self, property: Entity, value: String) -> &mut Self {
        self.queue(move |world: &mut World| {
            let text_input: Entity = {
                let mut system_state: SystemState<(
                    Query<&Children>,
                    Query<Entity, With<KeaTextInput>>,
                )> = SystemState::new(world);

                let mut result = Entity::PLACEHOLDER;
                let (parents, text_inputs) = system_state.get(world);
                for child in parents.iter_descendants(property) {
                    if text_inputs.contains(child) {
                        result = child;
                        break;
                    }
                }

                system_state.apply(world);

                result
            };

            if text_input != Entity::PLACEHOLDER {
                let mut commands = world.commands();
                commands.kea_text_input_set_text(text_input, value);
                world.flush();
            }
        });
        self
    }
}
