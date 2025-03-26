mod panel;
mod resources;
mod shapes;

use bevy::prelude::*;
pub use panel::Panel;
pub use panel::PanelOptions;
pub use resources::Resources;
pub use shapes::Shapes;

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        resources::initialize(app);
    }
}
