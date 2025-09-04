use bevy::prelude::*;
use lstalk::prelude::*;

mod lsp;
mod project;

pub use {
    lsp::LSPServiceResource,
    project::ProjectResource,
};

#[derive(Event)]
pub enum LSPEvent {
    Symbols,
}

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
    mut commands: Commands,
) {
    let result = lsp_resource.poll();

    let mut update_symbols = false;
    let mut symbols_result = None;

    let mut initialized = false;
    for item in result.items {
        match item.event {
            LanguageServerEvent::Initialized => {
                initialized = true;
            },
            LanguageServerEvent::RetrievedSymbols(symbols) => {
                update_symbols = true;
                symbols_result = Some(symbols);
                commands.trigger(LSPEvent::Symbols);
            },
        }
    }

    if initialized {
        let documents = project_resource.project.gather_source_files();
        lsp_resource.service.request_types(documents);
    }

    // Need to perfrom move here due to borrow occurring in for loop above.
    if update_symbols {
        lsp_resource.symbols = symbols_result;
    }
}
