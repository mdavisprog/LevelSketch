use bevy::prelude::*;
use kea::prelude::*;
use super::{
    CameraTools,
    FileTools,
    level::LevelTools,
    ShapesTools,
    types::TypesTool,
};

#[derive(Component)]
pub struct ToolsPanel {
    _private: (),
}

impl ToolsPanel {
    pub fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        KeaPanel::bundle(KeaPanelOptions {
            title: format!("Tools"),
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
            children![
                KeaExpander::bundle_with_header("File", FileTools::bundle()),
                KeaExpander::bundle_with_header("Camera", CameraTools::bundle()),
                KeaExpander::bundle_with_header("Level", LevelTools::bundle()),
                KeaExpander::bundle_with_header("Types", TypesTool::bundle()),
                KeaExpander::bundle_with_header("Shapes", ShapesTools::bundle()),
            ],
        )),
    )}
}
