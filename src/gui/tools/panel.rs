use bevy::prelude::*;
use kea::prelude::*;
use super::{
    CameraTools,
    FileTools,
    ShapesTools,
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
                KeaExpander::bundle("File", FileTools::bundle()),
                KeaExpander::bundle("Camera", CameraTools::bundle()),
                KeaExpander::bundle("Shapes", ShapesTools::bundle()),
            ],
        )),
    )}
}
