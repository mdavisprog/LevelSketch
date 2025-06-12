use bevy::prelude::*;
use crate::style;
use super::{
    component::{
        KeaList,
        KeaListBehavior,
    },
};

pub(super) struct Select {
    pub(super) list: Entity,
    pub(super) items: Vec<Entity>,
}

impl Command for Select {
    fn apply(self, world: &mut World) {
        let mut deselect = None;

        if let Ok(mut list_entity) = world.get_entity_mut(self.list) {
            if let Some(mut list) = list_entity.get_mut::<KeaList>() {
                match list.behavior {
                    KeaListBehavior::NoSelect => {},
                    KeaListBehavior::Select => {
                        deselect = Some(core::mem::take(&mut list.selected));
                        list.selected.extend_from_slice(&self.items);
                    },
                }
            }
        }

        if let Some(deselect) = deselect {
            for item in deselect {
                set_color(world, item, Color::srgba(0.0, 0.0, 0.0, 0.0));
            }
        }

        for item in self.items {
            set_color(world, item, style::colors::HIGHLIGHT);
        }
    }
}

pub(super) struct Deselect {
    pub(super) list: Entity,
    pub(super) items: Vec<Entity>,
}

impl Command for Deselect {
    fn apply(self, world: &mut World) {
        if let Ok(mut list_entity) = world.get_entity_mut(self.list) {
            if let Some(mut list) = list_entity.get_mut::<KeaList>() {
                list.selected.retain(|&value| !self.items.contains(&value));
            }
        }

        for item in self.items {
            set_color(world, item, Color::srgba(0.0, 0.0, 0.0, 0.0));
        }
    }
}

fn set_color(world: &mut World, item: Entity, color: Color) {
    let Ok(mut list_item) = world.get_entity_mut(item) else {
        return;
    };

    let Some(mut background_color) = list_item.get_mut::<BackgroundColor>() else {
        return;
    };

    *background_color = color.into();
}
