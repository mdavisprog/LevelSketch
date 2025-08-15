use bevy::prelude::*;

mod lsp;
mod project;

pub use {
    lsp::LSPServiceResource,
    project::{
        Project,
        ProjectResource,
    },
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<project::ProjectResource>()
            .init_resource::<lsp::LSPServiceResource>()
            .add_systems(Startup, setup)
            .add_systems(Update, update);
    }
}

fn setup(mut lsp_resource: ResMut<lsp::LSPServiceResource>) {
    lsp_resource.start();
}

fn update(lsp_resource: Res<lsp::LSPServiceResource>) {
    lsp_resource.poll();
}
