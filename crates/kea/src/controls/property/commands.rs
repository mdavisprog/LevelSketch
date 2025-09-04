use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use crate::controls::text::{
    KeaTextInput,
    KeaTextInputCommands,
};

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
