mod camera;
mod file;
mod panel;
mod shapes;
mod tools;

pub use camera::CameraTools;
pub use file::FileTools;
pub use panel::ToolsPanel;
pub use shapes::ShapesTools;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut bevy::app::App) {
        shapes::build(app);
    }
}
