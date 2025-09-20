use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use crate::style;
use super::{
    component::{
        KeaList,
        KeaListBehavior,
    },
};

pub trait KeaListCommandsExt {
    fn kea_list_select(&mut self, list: Entity, index: usize) -> &mut Self;
    fn kea_list_deselect(&mut self, list: Entity, index: usize) -> &mut Self;
}

impl<'w, 's> KeaListCommandsExt for Commands<'w, 's> {
    fn kea_list_select(&mut self, list: Entity, index: usize) -> &mut Self {
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&Children>,
                Query<&mut KeaList>,
                Commands,
            )> = SystemState::new(world);

            let (
                parents,
                mut lists,
                mut commands,
            ) = system_state.get_mut(world);

            let Ok(mut component) = lists.get_mut(list) else {
                warn!("Entity '{list}' is not a KeaList entity.");
                return;
            };

            let Ok(parent) = parents.get(list) else {
                warn!("KeaList '{list}' does not have any children.");
                return;
            };

            if index >= parent.len() {
                warn!("Given index is out-of-bounds: {index} >= {}.", parent.len());
                return;
            }

            match component.behavior {
                KeaListBehavior::NoSelect => {},
                KeaListBehavior::Select => {
                    for selected in &component.selected {
                        let Some(entity) = parent.get(*selected) else {
                            continue;
                        };

                        commands
                            .entity(*entity)
                            .insert(BackgroundColor(Color::NONE));
                    }

                    component.selected.clear();
                    component.selected.push(index);

                    let entity = parent[index];
                    commands
                        .entity(entity)
                        .insert(BackgroundColor(style::colors::HIGHLIGHT));
                },
            }

            system_state.apply(world);
        });
        self
    }

    fn kea_list_deselect(&mut self, list: Entity, index: usize) -> &mut Self {
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&Children>,
                Query<&mut KeaList>,
                Commands,
            )> = SystemState::new(world);

            let (
                parents,
                mut lists,
                mut commands,
            ) = system_state.get_mut(world);

            let Ok(mut component) = lists.get_mut(list) else {
                warn!("Entity '{list}' is not a KeaList entity.");
                return;
            };

            let Ok(parent) = parents.get(list) else {
                warn!("KeaList '{list}' does not have any children.");
                return;
            };

            if index >= parent.len() {
                warn!("Given index is out-of-bounds: {index} >= {}.", parent.len());
                return;
            }

            match component.behavior {
                KeaListBehavior::NoSelect => {},
                KeaListBehavior::Select => {
                    component.selected.retain(|element| *element != index);
                    let entity = parent[index];
                    commands
                        .entity(entity)
                        .insert(BackgroundColor(Color::NONE));
                },
            }

            system_state.apply(world);
        });
        self
    }
}
