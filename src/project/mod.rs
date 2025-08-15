use bevy::prelude::*;
use lstalk::prelude::*;

mod lsp;
mod project;

pub use {
    lsp::LSPServiceResource,
    project::ProjectResource,
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<project::ProjectResource>()
            .init_resource::<lsp::LSPServiceResource>()
            .add_systems(Update, update);
    }
}

fn update(
    project_resource: Res<ProjectResource>,
    mut lsp_resource: ResMut<LSPServiceResource>,
) {
    let result = lsp_resource.poll();

    let mut initialized = false;
    for item in result.items {
        match item.event {
            LanguageServerEvent::Initialized => {
                initialized = true;
            },
        }
    }

    if initialized {
        let documents = project_resource.project.gather_source_files();
        lsp_resource.service.request_types(documents);
    }
}
