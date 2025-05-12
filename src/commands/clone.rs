use bevy::prelude::*;

pub(crate) struct CloneHierarchy {
    /// The original entity to be cloned.
    origin: Entity,

    /// This entity should already be cloned by the clone_hierarchy command extension.
    target: Entity,
}

impl Command for CloneHierarchy {
    fn apply(self, world: &mut World) {
        self.clone(self.target, self.get_children(self.origin, world), world);

        world.flush();
    }
}

impl CloneHierarchy {
    pub(crate) fn new(origin: Entity, target: Entity) -> Self {
        Self {
            origin,
            target,
        }
    }

    fn clone(
        &self,
        target: Entity,
        children: Vec<Entity>,
        world: &mut World,
    ) {
        let pairs = {
            let mut result = Vec::<(Entity, Entity)>::new();

            let mut commands = world.commands();
            for child in children {
                let cloned = commands.entity(child).clone_and_spawn().id();
                commands.entity(target).add_child(cloned);
                result.push((child, cloned));
            }

            result
        };

        for pair in pairs {
            let descendants = self.get_children(pair.0, world);
            self.clone(pair.1, descendants, world);
        }
    }

    fn get_children(&self, origin: Entity, world: &mut World) -> Vec<Entity> {
        let mut result = Vec::<Entity>::new();

        if let Some(children) = world.entity(origin).get::<Children>() {
            for child in children {
                result.push(*child);
            }
        }

        result
    }
}
