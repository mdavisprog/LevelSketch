use bevy::prelude::*;

mod commands;
mod component;
mod systems;

pub use {
    commands::KeaPanelCommands,
    component::{
        KeaPanel,
        KeaPanelCloseBehavior,
        KeaPanelOptions,
    },
};

#[derive(Event)]
pub struct KeaPanelClose;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        systems::build(app);
    }
}
