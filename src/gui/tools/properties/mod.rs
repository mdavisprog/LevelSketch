use bevy::prelude::*;

mod property;
mod tool;
mod transform;

pub use tool::PropertiesTool;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        tool::build(app);
    }
}
