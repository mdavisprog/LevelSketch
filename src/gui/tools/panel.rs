use bevy::prelude::*;
use kea::prelude::*;
use super::{
    CameraTools,
    FileTools,
    level::LevelTools,
    ShapesTools,
    types::TypesTool,
};

#[derive(Component, Debug, PartialEq, Eq, Clone, Copy, Hash)]
pub enum ToolsPanelType {
    Project,
    Assets,
}

#[derive(Component)]
pub struct ToolsPanel {
    _private: (),
}

impl ToolsPanel {
    pub fn project_panel() -> impl Bundle {(
        Self::bundle("Project", children![
            KeaExpander::bundle_with_header("File", FileTools::bundle()),
            KeaExpander::bundle_with_header("Camera", CameraTools::bundle()),
            KeaExpander::bundle_with_header("Level", LevelTools::bundle()),
            KeaExpander::bundle_with_header("Types", TypesTool::bundle()),
        ]),
        ToolsPanelType::Project,
    )}

    pub fn assets_panel() -> impl Bundle {(
        Self::bundle("Assets", children![
            KeaExpander::bundle_with_header("Shapes", ShapesTools::bundle()),
        ]),
        ToolsPanelType::Assets,
        Visibility::Hidden,
    )}

    fn bundle(title: &str, bundles: impl Bundle) -> impl Bundle {(
        Self {
            _private: (),
        },
        KeaPanel::bundle(KeaPanelOptions {
            title: title.to_string(),
            position: Vec2::new(50.0, 100.0),
            size: Vec2::new(300.0, 400.0),
            close_behavior: KeaPanelCloseBehavior::Trigger,
        },
        (
            Node {
                flex_direction: FlexDirection::Column,
                row_gap: Val::Px(kea::style::properties::ROW_GAP),
                width: Val::Percent(100.0),
                height: Val::Percent(100.0),
                ..default()
            },
            KeaScrollable,
            bundles,
        )),
    )}
}
