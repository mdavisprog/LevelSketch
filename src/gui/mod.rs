use bevy::prelude::*;
use kea::prelude::*;

pub mod droppable;
pub mod panels;
pub mod style;

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

    commands.spawn(
        KeaPanel::bundle(KeaPanelOptions {
            title: format!("Tools"),
            position: Vec2::new(100.0, 100.0),
            size: Vec2::new(200.0, 400.0),
        },
        (
            Node {
                flex_direction: FlexDirection::Column,
                row_gap: Val::Px(kea::style::properties::ROW_GAP),
                width: Val::Percent(100.0),
                ..default()
            },
            children![
                KeaExpander::bundle("File", tools::FileTools::bundle()),
                KeaExpander::bundle("Camera", tools::CameraTools::bundle()),
            ],
        )),
    );
}
