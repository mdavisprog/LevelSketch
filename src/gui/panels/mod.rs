mod panel;
mod resources;

use bevy::prelude::*;
pub use panel::Panel;
pub use panel::PanelOptions;
pub use resources::Resources;

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        resources::initialize(app);
    }
}
