use bevy::prelude::*;

pub mod droppable;
pub mod panels;
pub mod style;

mod trail;
mod viewport;

//
// Types
//

pub struct GUIPlugin;

impl Plugin for GUIPlugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<State>()
            .add_plugins((
                panels::Plugin,
            ))
            .add_systems(Startup, setup);

        droppable::build(app);
        trail::build(app);
    }
}

#[derive(Resource)]
pub struct State {
    is_interacting: bool,
    did_drag: bool,
}

impl Default for State {
    fn default() -> Self {
        Self {
            is_interacting: false,
            did_drag: false,
        }
    }
}

impl State {
    pub fn is_interacting(&self) -> bool {
        self.is_interacting
    }
}

//
// Systems
//

fn setup(
    mut commands: Commands,
) {
    let mut entity = commands.spawn(viewport::Viewport);
    viewport::Viewport::observe(&mut entity);

    let bundle = panels::Shapes::bundle(Vec2::new(50.0, 50.0), &mut commands);
    commands.spawn(bundle);
}
