use bevy::prelude::*;
use bevy::ecs::system::SystemId;

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<DropInfo>();
}

#[derive(Component)]
pub struct Droppable {
    callback: SystemId,
}

impl Droppable {
    pub fn new(callback: SystemId) -> Self {
        Self {
            callback,
        }
    }

    pub fn invoke(&self, commands: &mut Commands) {
        commands.run_system(self.callback);
    }
}

#[derive(Resource)]
pub struct DropInfo {
    pub target: Entity,
}

impl Default for DropInfo {
    fn default() -> Self {
        Self {
            target: Entity::PLACEHOLDER,
        }
    }
}
