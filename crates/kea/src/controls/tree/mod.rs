use bevy::prelude::*;

mod commands;
mod events;
mod systems;
mod tree;

pub use {
    commands::KeaTreeCommands,
    events::{
        KeaTreeClick,
        KeaTreeHover,
    },
    tree::KeaTree,
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        systems::build(app);
    }
}
