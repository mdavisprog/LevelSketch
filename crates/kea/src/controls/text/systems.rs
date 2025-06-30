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
    KeaTextInputFormat,
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
    text_inputs: Query<&KeaTextInput>,
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

    let Ok(text_input) = text_inputs.get(resource.focused) else {
        events.clear();
        return;
    };

    for event in events.read() {
        if event.state == ButtonState::Pressed {
            match event.key_code {
                KeyCode::Enter | KeyCode::NumpadEnter => {
                    text.0 = text_input.format.convert(&text.0);
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

                    let string = pending_text.to_string();
                    let delta = match text_input.format {
                        KeaTextInputFormat::Default => string,
                        KeaTextInputFormat::Numbers(_) => {
                            if pending_text.parse::<f32>().is_ok() {
                                string
                            } else if pending_text == "." {
                                if !text.0.contains(".") {
                                    string
                                } else {
                                    format!("")
                                }
                            } else if pending_text == "-" {
                                if text.0.is_empty() {
                                    string
                                } else {
                                    format!("")
                                }
                            } else {
                                format!("")
                            }
                        },
                    };

                    text.0 += &delta;
                },
            }
        }
    }
}
