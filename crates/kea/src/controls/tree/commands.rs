use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use super::{
    systems::UpdateTree,
    tree::{
        KeaTree,
        KeaTreeRoot,
    },
};

pub trait KeaTreeCommands {
    fn kea_tree_spawn_child(&mut self, tree: Entity, bundle: impl Bundle) -> EntityCommands<'_>;
}

impl<'w, 's> KeaTreeCommands for Commands<'w, 's> {
    fn kea_tree_spawn_child(&mut self, tree: Entity, bundle: impl Bundle) -> EntityCommands<'_> {
        let entity = self.spawn(bundle).id();
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&KeaTreeRoot>,
                Query<&KeaTree>,
                Query<&Children>,
                Commands,
            )> = SystemState::new(world);

            let (
                roots,
                trees,
                parents,
                mut commands,
            ) = system_state.get_mut(world);

            if !roots.contains(tree) {
                return;
            };

            for child in parents.iter_descendants(tree) {
                if !trees.contains(child) {
                    continue;
                }

                commands
                    .entity(child)
                    .add_child(entity);
                commands.trigger(UpdateTree);

                break;
            }

            system_state.apply(world);
        });
        self.entity(entity)
    }
}
