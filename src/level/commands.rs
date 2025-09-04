use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use crate::entitiy::EntityProperties;
use super::{
    events::LevelEventAddEntity,
    level::Level,
};

pub trait LevelCommands {
    fn spawn_level(&mut self, bundle: impl Bundle) -> EntityCommands<'_>;
}

impl<'w, 's> LevelCommands for Commands<'w, 's> {
    fn spawn_level(&mut self, bundle: impl Bundle) -> EntityCommands<'_> {
        let entity = self.spawn(bundle).id();
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<Entity, With<Level>>,
                Commands,
                EventWriter<LevelEventAddEntity>,
            )> = SystemState::new(world);

            let (
                levels,
                mut commands,
                mut events,
            ) = system_state.get_mut(world);

            let Ok(level) = levels.single() else {
                panic!("No single Level entity found!");
            };

            commands
                .entity(level)
                .add_child(entity);

            commands
                .entity(entity)
                .insert_if_new(EntityProperties::new());

            events.write(LevelEventAddEntity {
                entity,
            });

            system_state.apply(world);
        });
        self.entity(entity)
    }
}
