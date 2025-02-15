use bevy::prelude::*;

pub mod menu;

//
// Public API
//

pub fn close_menus(commands: &mut Commands) {
    menu::close_menus(commands);
}

//
// Types
//

pub struct GUIPlugin;

impl Plugin for GUIPlugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<State>()
            .add_plugins(menu::Plugin)
            .add_systems(Startup, setup);
    }
}

#[derive(Resource)]
pub struct State {
    is_interacting: bool,
}

impl Default for State {
    fn default() -> Self {
        Self {
            is_interacting: false,
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
    commands.spawn((
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            ..default()
        },
        GlobalZIndex(-1),
        PickingBehavior {
            should_block_lower: false,
            is_hoverable: true,
        }
    ))
    .observe(on_root_over)
    .observe(on_root_out);
}

fn on_root_over(
    _: Trigger<Pointer<Over>>,
    mut state: ResMut<State>,
) {
    state.is_interacting = false;
}

fn on_root_out(
    _: Trigger<Pointer<Out>>,
    mut state: ResMut<State>,
) {
    state.is_interacting = true;
}
