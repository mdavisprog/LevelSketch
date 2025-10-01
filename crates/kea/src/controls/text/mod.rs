use bevy::prelude::*;

mod commands;
mod cursor;
mod document;
mod events;
mod input;
mod resources;
mod systems;

pub use {
    commands::KeaTextInputCommands,
    events::{
        KeaTextInputConfirm,
        KeaTextInputUnfocus,
    },
    input::{
        KeaTextInput,
        KeaTextInputFormat,
        KeaTextInputFormatNumber,
        KeaTextInputFormatNumberType,
    },
    resources::KeaTextInputResource,
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<resources::KeaTextInputResource>()
            .add_event::<events::KeaTextInputSetCursorPosition>();

        systems::build(app);
    }
}
