use bevy::prelude::*;

mod component;
mod systems;

pub use component::{
    KeaPanel,
    KeaPanelCloseBehavior,
    KeaPanelOptions,
};

#[derive(Event)]
pub struct KeaPanelClose;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        systems::build(app);
    }
}
