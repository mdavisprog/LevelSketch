use bevy::prelude::*;

mod tools;

pub use tools::ToolsMenu;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        tools::build(app);
    }
}
