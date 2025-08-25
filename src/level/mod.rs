use bevy::prelude::*;

mod commands;
mod events;
mod level;
mod systems;

pub use {
    commands::LevelCommands,
    events::LevelEventAddEntity,
    level::Level,
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.add_event::<LevelEventAddEntity>();

        systems::build(app);
    }
}
