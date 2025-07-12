use bevy::prelude::*;

mod animatable;
mod component;
mod events;
mod systems;

pub use {
    component::{
        KeaAnimation,
        KeaAnimationClip,
    },
    events::KeaAnimationComplete,
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        systems::build(app);
    }
}
