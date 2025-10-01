use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use crate::style;
use super::{
    cursor::Cursor,
    document::{
        Document,
        DocumentContents,
    },
    events::KeaTextInputSetCursorPosition,
    input::KeaTextInput,
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
                Query<&mut Text, With<DocumentContents>>,
            )> = SystemState::new(world);

            let (
                parents,
                text_inputs,
                mut texts,
            ) = system_state.get_mut(world);

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
        Query<&Children>,
        Query<&Document>,
        Query<&mut BackgroundColor>,
        Query<&mut Node>,
        Query<&mut Visibility, With<Cursor>>,
        ResMut<KeaTextInputResource>,
        EventWriter<KeaTextInputSetCursorPosition>,
    )> = SystemState::new(world);

    let (
        parents,
        documents,
        mut background_colors,
        mut nodes,
        mut cursors,
        mut resource,
        mut events,
    ) = system_state.get_mut(world);

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

    let mut document = Entity::PLACEHOLDER;
    let mut cursor = Entity::PLACEHOLDER;

    for child in parents.iter_descendants(text_input) {
        if documents.contains(child) {
            document = child;
        } else if cursors.contains(child) {
            cursor = child;
        }
    }

    // Update the text justification of the Document based on focus state.
    if let Ok(mut node) = nodes.get_mut(document) {
        node.justify_content = if focused {
            JustifyContent::Start
        } else {
            JustifyContent::Center
        };
    }

    // Show/hide the cursor entity and set its index to the end of the text.
    if let Ok(mut visibility) = cursors.get_mut(cursor) {
        *visibility = if focused {
            Visibility::Inherited
        } else {
            Visibility::Hidden
        };

        events.write(KeaTextInputSetCursorPosition {
            text_input,
            index: usize::MAX,
        });
    }

    system_state.apply(world);
}
