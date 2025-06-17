use bevy::prelude::*;

mod commands;
mod component;
mod events;
mod extensions;
mod systems;

pub use {
    component::{
        KeaList,
        KeaListBehavior,
        KeaListLabelItems,
    },
    events::{
        KeaListHover,
        KeaListSelect,
    },
    extensions::KeaListCommandsExt,
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        systems::build(app);
    }
}
