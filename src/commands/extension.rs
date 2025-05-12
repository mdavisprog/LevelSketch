use bevy::prelude::*;
use super::clone::CloneHierarchy;

pub trait CommandsExt {
    fn clone_hierarchy(&mut self, entity: Entity) -> EntityCommands<'_>;
}

impl<'w, 's> CommandsExt for Commands<'w, 's> {
    fn clone_hierarchy(&mut self, entity: Entity) -> EntityCommands<'_> {
        let cloned = self.entity(entity).clone_and_spawn().id();
        self.queue(CloneHierarchy::new(entity, cloned));
        self.entity(cloned)
    }
}
