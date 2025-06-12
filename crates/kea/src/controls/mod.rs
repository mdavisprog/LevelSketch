use bevy::prelude::*;

pub(super) mod anchors;
pub(super) mod button;
pub(super) mod image;
pub(super) mod panel;
pub(super) mod sizer;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.add_plugins((
            button::Plugin,
            panel::Plugin,
        ));

        image::build(app);
        sizer::build(app);
    }
}
