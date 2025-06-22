use bevy::prelude::*;

#[derive(Event)]
pub struct KeaTextInputConfirm {
    pub text: String,
}
