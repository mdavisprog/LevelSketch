use bevy::prelude::*;
use crate::controls::checkbox::{
    KeaCheckbox,
    KeaCheckboxState,
};
use super::systems::on_checkbox;

#[derive(Component)]
pub struct KeaPropertyBoolean {
    pub(super) value: bool,
}

impl KeaPropertyBoolean {
    pub fn bundle(label: &str, value: bool) -> impl Bundle {(
        Self {
            value,
        },
        KeaCheckbox::bundle_with_state(
            on_checkbox,
            label,
            if value { KeaCheckboxState::Checked } else { KeaCheckboxState::Unchecked },
        ),
    )}

    pub fn value(&self) -> bool {
        self.value
    }
}
