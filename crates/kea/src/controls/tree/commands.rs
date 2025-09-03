use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use super::tree::KeaTree;

pub trait KeaTreeCommands {
    fn kea_tree_spawn_child(&mut self, tree: Entity, bundle: impl Bundle) -> EntityCommands<'_>;
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
}
