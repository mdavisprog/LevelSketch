use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use crate::style;
use super::tree::{
    DataItem,
    KeaTree,
};

pub trait KeaTreeCommands {
    fn kea_tree_spawn_child(&mut self, tree: Entity, bundle: impl Bundle) -> EntityCommands<'_>;
    fn kea_tree_set_selected(&mut self, tree: Entity, selected: bool) -> &mut Self;
}

impl<'w, 's> KeaTreeCommands for Commands<'w, 's> {
    fn kea_tree_spawn_child(&mut self, tree: Entity, bundle: impl Bundle) -> EntityCommands<'_> {
        let entity = self.spawn(bundle).id();
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&KeaTree>,
                Commands,
            )> = SystemState::new(world);

            let (
                trees,
                mut commands,
            ) = system_state.get_mut(world);

            let Ok(component) = trees.get(tree) else {
                return;
            };

            commands
                .entity(component.contents)
                .add_child(entity);

            system_state.apply(world);
        });
        self.entity(entity)
    }

    fn kea_tree_set_selected(&mut self, tree: Entity, selected: bool) -> &mut Self {
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&Children>,
                Query<&DataItem>,
                Query<&mut KeaTree>,
                Commands,
            )> = SystemState::new(world);

            let (
                parents,
                data_items,
                mut trees,
                mut commands,
            ) = system_state.get_mut(world);

            let Ok(mut component) = trees.get_mut(tree) else {
                return;
            };

            component.selected = selected;

            for child in parents.iter_descendants(tree) {
                if !data_items.contains(child) {
                    continue;
                }

                let color = if selected {
                    style::colors::HIGHLIGHT
                } else {
                    style::colors::BACKGROUND
                };

                commands
                    .entity(child)
                    .insert(BackgroundColor(color));
                break;
            }

            system_state.apply(world);
        });
        self
    }
}
