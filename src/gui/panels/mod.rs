pub mod events;
mod panel;
mod resources;

use bevy::prelude::*;
pub use panel::Panel;
pub use resources::Resources;

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut bevy::app::App) {
        app
            .add_event::<events::Open>()
            .add_systems(Update, events::on_open);

        resources::initialize(app);
    }
}
