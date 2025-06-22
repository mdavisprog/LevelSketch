use bevy::{
    diagnostic::FrameCount,
    input::{
        ButtonState,
        keyboard::KeyboardInput,
    },
    prelude::*,
};
use super::{
    events::KeaTextInputConfirm,
    input::TextInput,
    KeaTextInput,
    KeaTextInputCommands,
    KeaTextInputResource,
};

pub(super) fn build(app: &mut App) {
    app
        .add_observer(on_click)
        .add_systems(Update, keyboard_input);
}

fn on_click(
    trigger: Trigger<Pointer<Click>>,
    text_inputs: Query<&KeaTextInput>,
    frame: Res<FrameCount>,
    mut resource: ResMut<KeaTextInputResource>,
    mut commands: Commands,
    mut last_frame: Local<u32>,
) {
    let target = trigger.target();

    if text_inputs.contains(target) {
        resource.focused = target;
        *last_frame = frame.0;
        commands.kea_text_input_set_focus(target, true);
    } else {
        if *last_frame < frame.0 {
            if resource.focused != Entity::PLACEHOLDER {
                commands.kea_text_input_set_focus(resource.focused, false);
            }

            resource.focused = Entity::PLACEHOLDER;
        }
    }
}

fn keyboard_input(
    resource: Res<KeaTextInputResource>,
    parents: Query<&Children>,
    mut events: EventReader<KeyboardInput>,
    mut texts: Query<&mut Text, With<TextInput>>,
    mut commands: Commands,
) {
    let mut text_entity = Entity::PLACEHOLDER;
    for child in parents.iter_descendants(resource.focused) {
        if texts.contains(child) {
            text_entity = child;
            break;
        }
    }

    let Ok(mut text) = texts.get_mut(text_entity) else {
        events.clear();
        return;
    };

    for event in events.read() {
        if event.state == ButtonState::Pressed {
            match event.key_code {
                KeyCode::Enter | KeyCode::NumpadEnter => {
                    commands
                        .kea_text_input_set_focus(resource.focused, false)
                        .trigger_targets(KeaTextInputConfirm {
                            text: text.0.clone(),
                        }, resource.focused);
                },
                KeyCode::Backspace => {
                    text.0.pop();
                },
                _ => {
                    let Some(pending_text) = &event.text else {
                        continue;
                    };

                    text.0 += pending_text.as_str();
                },
            }
        }
    }
}
