use bevy::prelude::*;
use crate::project::{
    LSPServiceResource,
    ProjectResource,
};
use kea::prelude::*;
use rfd::FileDialog;
use super::{
    panel::ToolsPanelType,
    tools::Tools,
};

#[derive(Component)]
#[require(Tools)]
pub struct FileTools {
    _private: (),
}

impl FileTools {
    pub fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            (
                KeaButton::label_bundle( "Open Project", on_open_project),
            ),
            (
                KeaButton::label_bundle("Settings", on_settings),
            ),
            (
                KeaButton::label_bundle( "Quit", on_quit),
            ),
        ]
    )}
}

fn on_open_project(
    _: Trigger<KeaButtonClick>,
    mut lsp: ResMut<LSPServiceResource>,
    mut project_resource: ResMut<ProjectResource>
) {
    let Some(response) = FileDialog::new().pick_folder() else {
        return;
    };

    let Some(path) = response.to_str() else {
        return;
    };

    if let Err(error) = project_resource.project.open(path) {
        warn!("Failed to load project: {error:?}");
        return;
    }

    lsp.start(path);
}

fn on_settings(
    _: Trigger<KeaButtonClick>,
    panel_types: Query<(Entity, &ToolsPanelType)>,
    mut commands: Commands,
) {
    for (entity, panel) in panel_types {
        if *panel == ToolsPanelType::Settings {
            commands
                .kea_panel_focus(entity)
                .entity(entity)
                .insert(Visibility::Visible);
            break;
        }
    }
}

fn on_quit(
    _: Trigger<KeaButtonClick>,
    mut events: EventWriter<AppExit>,
) {
    events.write(AppExit::Success);
}
