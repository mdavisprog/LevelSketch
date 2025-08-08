use bevy::prelude::*;

mod project;

pub use {
    project::{
        Project,
        ProjectResource,
    },
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<project::ProjectResource>();
    }
}
