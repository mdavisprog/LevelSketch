use bevy::prelude::*;
use super::commands::ChangeLabel;

pub trait KeaButtonCommandsExt {
    fn kea_button_change_label(&mut self, button: Entity, label: &str);
}

impl<'w, 's> KeaButtonCommandsExt for Commands<'w, 's> {
    fn kea_button_change_label(&mut self, button: Entity, label: &str) {
        self.queue(ChangeLabel {
            entity: button,
            label: label.to_string(),
        });
    }
}
