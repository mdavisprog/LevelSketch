use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use crate::style;
use super::{
    input::{
        KeaTextInput,
        TextInput,
    },
    resources::KeaTextInputResource,
};

pub trait KeaTextInputCommands {
    fn kea_text_input_set_focus(&mut self, text_input: Entity, focused: bool) -> &mut Self;
    fn kea_text_input_set_text(&mut self, text_input: Entity, text: String) -> &mut Self;
}

impl<'w, 's> KeaTextInputCommands for Commands<'w, 's> {
    fn kea_text_input_set_focus(&mut self, text_input: Entity, focused: bool) -> &mut Self {
        self.queue(move |world: &mut World| text_input_focus(world, text_input, focused));
        self
    }

    fn kea_text_input_set_text(&mut self, text_input: Entity, text: String) -> &mut Self {
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&Children>,
                Query<&KeaTextInput>,
                Query<&mut Text, With<TextInput>>,
            )> = SystemState::new(world);

            let (parents, text_inputs, mut texts) = system_state.get_mut(world);

            let Ok(input) = text_inputs.get(text_input) else {
                return;
            };

            for child in parents.iter_descendants(text_input) {
                let Ok(mut text_object) = texts.get_mut(child) else {
                    continue;
                };

                text_object.0 = input.format.convert(&text);
                break;
            }

            system_state.apply(world);
        });
        self
    }
}

fn text_input_focus(
    world: &mut World,
    text_input: Entity,
    focused: bool,
) {
    let mut system_state: SystemState<(
        Query<&mut BackgroundColor>,
        Query<&mut Node>,
        ResMut<KeaTextInputResource>,
    )> = SystemState::new(world);

    let (mut background_colors, mut nodes, mut resource) = system_state.get_mut(world);

    resource.focused = if focused {
        text_input
    } else {
        Entity::PLACEHOLDER
    };

    if let Ok(mut background_color) = background_colors.get_mut(text_input) {
        *background_color = if focused {
            style::colors::TEXT_INPUT_EDIT.into()
        } else {
            style::colors::BUTTON_BACKGROUND.into()
        };
    }

    if let Ok(mut node) = nodes.get_mut(text_input) {
        node.justify_content = if focused {
            JustifyContent::Start
        } else {
            JustifyContent::Center
        };
    }

    system_state.apply(world);
}
