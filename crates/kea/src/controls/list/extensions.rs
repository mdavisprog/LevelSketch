use bevy::prelude::*;
use super::commands::{
    Select,
    Deselect,
};

pub trait KeaListCommandsExt {
    fn kea_list_select(&mut self, list: Entity, items: &[Entity]) -> &mut Self;
    fn kea_list_deselect(&mut self, list: Entity, items: &[Entity]) -> &mut Self;
}

impl<'w, 's> KeaListCommandsExt for Commands<'w, 's> {
    fn kea_list_select(&mut self, list: Entity, items: &[Entity]) -> &mut Self {
        self.queue(Select {
            list,
            items: items.to_vec(),
        });

        self
    }

    fn kea_list_deselect(&mut self, list: Entity, items: &[Entity]) -> &mut Self {
        self.queue(Deselect {
            list,
            items: items.to_vec(),
        });

        self
    }
}
