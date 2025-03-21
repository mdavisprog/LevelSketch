pub mod events;
mod panel;

use bevy::prelude::*;
use crate::svg;
pub use panel::Panel;
use super::icons;

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut bevy::app::App) {
        app
            .add_event::<events::Open>()
            .add_systems(Update, events::on_open);
    }
}
