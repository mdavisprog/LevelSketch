use bevy::prelude::*;

mod settings;

pub use settings::SettingsTools;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, _app: &mut App) {
    }
}
