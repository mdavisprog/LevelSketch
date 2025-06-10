use bevy::prelude::*;

mod component;
mod events;
mod systems;

pub use {
    component::KeaButton,
    events::KeaButtonClick,
};

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .insert_resource(State {
                hot_button: None,
            })
            .add_observer(systems::global_mouse_released);
    }
}

///
/// State
///
/// Keeps track of information related to all buttons.
///
#[derive(Resource)]
struct State {
    hot_button: Option<Entity>,
}
