use bevy::{
    input::{
        ButtonState,
        keyboard::KeyboardInput,
    },
    prelude::*,
    text::TextLayoutInfo,
    ui::UiSystem,
};
use super::{
    cursor::Cursor,
    document::DocumentContents,
    events::KeaTextInputSetCursorPosition,
    KeaTextInput,
    KeaTextInputCommands,
    KeaTextInputFormat,
    resources::{
        FocusState,
        KeaTextInputResource,
    },
};

pub(super) fn build(app: &mut App) {
    app
        .add_observer(on_click)
        .add_systems(Update,
            (
                keyboard_input,
                update_focused,
            ))
        .add_systems(PostUpdate, set_cursor_position.after(UiSystem::PostLayout));
}

/// This trigger needs to be handled globally so that unfocused events can be detected
/// if the user has clicked outside of all KeaTextInput controls.
fn on_click(
    trigger: Trigger<Pointer<Click>>,
    text_inputs: Query<&KeaTextInput>,
    mut resource: ResMut<KeaTextInputResource>,
) {
    let target = trigger.target();

    match resource.focus_state {
        FocusState::None => {
            resource.focus_state = if text_inputs.contains(target) {
                FocusState::Pending(target)
            } else {
                FocusState::Pending(Entity::PLACEHOLDER)
            }
        },
        FocusState::Pending(_) => {
            if text_inputs.contains(target) {
                resource.focus_state = FocusState::Pending(target);
            }
        }
    }
}

fn keyboard_input(
    parents: Query<&Children>,
    text_inputs: Query<&KeaTextInput>,
    mut key_events: EventReader<KeyboardInput>,
    mut cursor_events: EventWriter<KeaTextInputSetCursorPosition>,
    mut texts: Query<&mut Text, With<DocumentContents>>,
    mut cursors: Query<&mut Cursor>,
    mut resource: ResMut<KeaTextInputResource>,
) {
    let mut text_entity = Entity::PLACEHOLDER;
    let mut cursor_entity = Entity::PLACEHOLDER;

    for child in parents.iter_descendants(resource.focused) {
        if texts.contains(child) {
            text_entity = child;
        } else if cursors.contains(child) {
            cursor_entity = child;
        }
    }

    let Ok(mut text) = texts.get_mut(text_entity) else {
        key_events.clear();
        return;
    };

    let Ok(mut cursor) = cursors.get_mut(cursor_entity) else {
        key_events.clear();
        return;
    };

    let Ok(text_input) = text_inputs.get(resource.focused) else {
        key_events.clear();
        return;
    };

    for event in key_events.read() {
        if event.state == ButtonState::Pressed {
            match event.key_code {
                KeyCode::Enter | KeyCode::NumpadEnter => {
                    resource.focus_state = FocusState::Pending(Entity::PLACEHOLDER);
                },
                KeyCode::Backspace => {
                    if cursor.index < text.0.len() {
                        if cursor.index > 0 {
                            text.0.remove(cursor.index - 1);
                            cursor.index = cursor.index.saturating_sub(1);
                        }
                    } else {
                        text.0.pop();
                    }
                    cursor_events.write(KeaTextInputSetCursorPosition {
                        text_input: resource.focused,
                        index: cursor.index,
                    });
                },
                KeyCode::Delete => {
                    if cursor.index >= text.0.len() {
                        continue;
                    }

                    text.0.remove(cursor.index);

                    cursor_events.write(KeaTextInputSetCursorPosition {
                        text_input: resource.focused,
                        index: cursor.index,
                    });
                },
                KeyCode::ArrowLeft | KeyCode::ArrowRight => {
                    let right = event.key_code == KeyCode::ArrowRight;
                    cursor.index = if right {
                        cursor.index + 1
                    } else {
                        cursor.index.saturating_sub(1)
                    };

                    cursor_events.write(KeaTextInputSetCursorPosition {
                        text_input: resource.focused,
                        index: cursor.index,
                    });
                }
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

                    text.0.insert_str(cursor.index, &delta);
                    cursor.index += delta.len();
                    cursor_events.write(KeaTextInputSetCursorPosition {
                        text_input: resource.focused,
                        index: cursor.index,
                    });
                },
            }
        }
    }
}

fn set_cursor_position(
    parents: Query<&Children>,
    texts: Query<(&Text, &TextLayoutInfo)>,
    mut events: EventReader<KeaTextInputSetCursorPosition>,
    mut cursors: Query<(&mut Node, &mut Cursor)>,
) {
    for event in events.read() {
        let mut cursor_entity = Entity::PLACEHOLDER;
        let mut text_entity = Entity::PLACEHOLDER;

        for child in parents.iter_descendants(event.text_input) {
            if cursors.contains(child) {
                cursor_entity = child;
            } else if texts.contains(child) {
                text_entity = child;
            }
        }

        if let Ok((text, layout)) = texts.get(text_entity) {
            if let Ok((mut cursor_node, mut cursor)) = cursors.get_mut(cursor_entity) {
                cursor.index = event.index.min(text.0.len());

                let position = if let Some(glyph) = layout.glyphs.get(cursor.index) {
                    Rect::from_center_size(glyph.position, glyph.size).min
                } else {
                    Vec2::new(layout.size.x, 0.0)
                };

                cursor_node.left = Val::Px(position.x);
            }
        }
    }
}

fn update_focused(
    mut resource: ResMut<KeaTextInputResource>,
    mut commands: Commands,
) {
    match resource.focus_state {
        FocusState::Pending(pending) => {
            if pending != resource.focused {
                if resource.focused != Entity::PLACEHOLDER {
                    commands.kea_text_input_set_focus(resource.focused, false);
                }

                resource.focused = pending;

                if resource.focused != Entity::PLACEHOLDER {
                    commands.kea_text_input_set_focus(resource.focused, true);
                }
            }

            resource.focus_state = FocusState::None;
        },
        FocusState::None => {},
    }
}
