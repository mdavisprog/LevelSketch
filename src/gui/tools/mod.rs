mod camera;
mod file;
mod level;
mod panel;
mod shapes;
mod tools;
mod types;

pub use {
    camera::CameraTools,
    file::FileTools,
    panel::{
        ToolsPanel,
        ToolsPanelType,
    },
    shapes::ShapesTools,
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut bevy::app::App) {
        level::build(app);
        shapes::build(app);
        types::build(app);
    }
}
