use bevy::prelude::*;
use bevy::render::camera::NormalizedRenderTarget;

pub mod buttonex;
pub mod icons;
pub mod menu;
pub mod panels;
pub mod style;

mod sizer;

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
            .init_resource::<buttonex::State>()
            .init_resource::<icons::Icons>()
            .add_plugins((
                menu::Plugin,
                panels::Plugin,
            ))
            .add_systems(Startup, setup)
            .add_observer(sizer::Sizer::on_added);
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
    asset_server: Res<AssetServer>,
    mut commands: Commands,
    mut icons: ResMut<icons::Icons>,
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
    .observe(on_root_out)
    .observe(on_root_down)
    .observe(on_root_click)
    .observe(on_root_drag_start);

    buttonex::ButtonEx::initialize(&mut commands);

    icons.initialize(&asset_server);
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

fn on_root_down(
    trigger: Trigger<Pointer<Down>>,
    mut commands: Commands,
    mut state: ResMut<State>,
) {
    close_menus(&mut commands);

    if trigger.button != PointerButton::Secondary {
        return;
    }

    state.did_drag = false;
}

fn on_root_click(
    trigger: Trigger<Pointer<Click>>,
    state: Res<State>,
    windows: Query<&Window>,
    mut events: EventWriter<panels::events::Open>,
) {
    if trigger.button != PointerButton::Secondary {
        return;
    }

    if !state.did_drag {
        let window_entity = match trigger.pointer_location.target {
            NormalizedRenderTarget::Window(value) => Some(value.entity()),
            _ => None,
        };

        let Some(window_entity) = window_entity else {
            return;
        };

        let Ok(window) = windows.get(window_entity) else {
            return;
        };

        let Some(cursor_position) = window.cursor_position() else {
            return;
        };

        events.send(panels::events::Open {
            position: cursor_position,
            title: format!("Panel"),
        });
    }
}

fn on_root_drag_start(
    trigger: Trigger<Pointer<DragStart>>,
    mut state: ResMut<State>,
) {
    if trigger.button != PointerButton::Secondary {
        return;
    }

    state.did_drag = true;
}
