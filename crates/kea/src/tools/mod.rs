use bevy::prelude::*;

mod inspector;

pub use inspector::KeaInspector;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.add_plugins(inspector::Plugin);
    }
}
