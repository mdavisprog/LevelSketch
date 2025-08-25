use bevy::prelude::*;

#[derive(Event)]
pub struct LevelEventAddEntity {
    pub entity: Entity,
}
