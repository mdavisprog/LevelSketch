use bevy::prelude::*;

mod item;
mod menus;
mod tools;
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
                menus::Plugin,
                tools::Plugin,
            ))
            .add_systems(Startup, setup);

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

    commands.spawn(
        tools::ToolsPanel::bundle()
    );
}
