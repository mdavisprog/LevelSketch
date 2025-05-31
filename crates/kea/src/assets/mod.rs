use bevy::prelude::*;

pub(crate) mod svg;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.add_plugins(svg::Plugin);
    }
}
