use bevy::prelude::*;

pub(super) mod anchors;
pub(super) mod button;
pub(super) mod checkbox;
pub(super) mod expander;
pub(super) mod image;
pub(super) mod label;
pub(super) mod list;
pub(super) mod panel;
pub(super) mod popup;
pub(super) mod property;
pub(super) mod scroll;
pub(super) mod separator;
pub(super) mod sizer;
pub(super) mod text;
pub(super) mod tree;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.add_plugins((
            button::Plugin,
            list::Plugin,
            panel::Plugin,
            popup::Plugin,
            scroll::Plugin,
            text::Plugin,
            tree::Plugin,
        ));

        checkbox::build(app);
        image::build(app);
        sizer::build(app);
    }
}
