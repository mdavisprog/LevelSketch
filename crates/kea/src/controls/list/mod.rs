use bevy::prelude::*;

mod commands;
mod component;
mod events;
mod systems;

pub use {
    commands::KeaListCommandsExt,
    component::{
        KeaList,
        KeaListBehavior,
        KeaListLabelItems,
    },
    events::{
        KeaListHover,
        KeaListSelect,
    },
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        systems::build(app);
    }
}
