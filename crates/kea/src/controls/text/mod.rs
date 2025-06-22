use bevy::prelude::*;

mod commands;
mod events;
mod input;
mod resources;
mod systems;

pub use {
    commands::KeaTextInputCommands,
    events::KeaTextInputConfirm,
    input::KeaTextInput,
    resources::KeaTextInputResource,
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<resources::KeaTextInputResource>();

        systems::build(app);
    }
}
