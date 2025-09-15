use bevy::prelude::*;

mod commands;
mod popup;
mod systems;

pub use {
    commands::KeaPopupCommands,
    popup::{
        KeaPopupPosition,
        KeaPopupSize,
        KeaPopupState,
    },
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<KeaPopupState>();

        systems::build(app);
    }
}
