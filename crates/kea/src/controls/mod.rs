use bevy::prelude::*;

pub(super) mod anchors;
pub(super) mod button;
pub(super) mod checkbox;
pub(super) mod image;
pub(super) mod list;
pub(super) mod panel;
pub(super) mod scrollable;
pub(super) mod separator;
pub(super) mod sizer;
pub(super) mod text;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.add_plugins((
            button::Plugin,
            list::Plugin,
            panel::Plugin,
            text::Plugin,
        ));

        checkbox::build(app);
        image::build(app);
        sizer::build(app);
    }
}
