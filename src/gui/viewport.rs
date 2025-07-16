use bevy::{
    prelude::*,
    render::camera::NormalizedRenderTarget,
};
use super::State;

/// UI Node that rests on top of the world viewport and below all other GUI nodes.
/// This node acts as a catch all for any interaction with viewport.
#[derive(Component)]
#[require(
    Node = Self::node(),
    GlobalZIndex(-1),
    Pickable = Self::pickable(),
)]
pub struct Viewport;

impl Viewport {
    pub fn observe(commands: &mut EntityCommands) {
        commands
            .observe(Self::on_over)
            .observe(Self::on_out)
            .observe(Self::on_down)
            .observe(Self::on_click);
    }

    fn on_over(
        _: Trigger<Pointer<Over>>,
        mut state: ResMut<State>,
    ) {
        state.is_interacting = false;
    }
    
    fn on_out(
        _: Trigger<Pointer<Out>>,
        mut state: ResMut<State>,
    ) {
        state.is_interacting = true;
    }
    
    fn on_down(
        trigger: Trigger<Pointer<Pressed>>,
        mut state: ResMut<State>,
    ) {
        if trigger.button != PointerButton::Secondary {
            return;
        }
    
        state.did_drag = false;
    }
    
    fn on_click(
        trigger: Trigger<Pointer<Click>>,
        state: Res<State>,
        windows: Query<&Window>,
        mut _commands: Commands,
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
    
            let Some(_cursor_position) = window.cursor_position() else {
                return;
            };
        }
    }

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            ..default()
        }
    }

    fn pickable() -> Pickable {
        Pickable { 
            should_block_lower: false,
            is_hoverable: true,
        }
    }
}
