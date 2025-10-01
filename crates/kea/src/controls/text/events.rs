use bevy::prelude::*;

#[derive(Event)]
pub struct KeaTextInputConfirm {
    pub text: String,
}

#[derive(Event)]
pub struct KeaTextInputUnfocus;

#[derive(Event)]
pub(super) struct KeaTextInputSetCursorPosition {
    pub text_input: Entity,
    pub index: usize,
}
