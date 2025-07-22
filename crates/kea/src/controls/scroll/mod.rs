use bevy::prelude::*;

mod events;
mod resources;
mod scrollable;
mod scrollbar;
mod systems;

pub use scrollable::KeaScrollable;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<resources::KeaScrollableResource>();

        systems::build(app);
    }
}
