use bevy::{
    prelude::*,
    render::camera::NormalizedRenderTarget,
};
use super::{
    droppable::{
        Droppable,
        DropInfo,
    },
    panels,
    State,
};

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
            .observe(Self::on_click)
            .observe(Self::on_drag_start)
            .observe(Self::on_drag_enter)
            .observe(Self::on_drag_over)
            .observe(Self::on_drag_drop);
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
        mut commands: Commands,
        mut state: ResMut<State>,
    ) {
        super::close_menus(&mut commands);
    
        if trigger.button != PointerButton::Secondary {
            return;
        }
    
        state.did_drag = false;
    }
    
    fn on_click(
        trigger: Trigger<Pointer<Click>>,
        state: Res<State>,
        windows: Query<&Window>,
        mut commands: Commands,
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

            let bundle = panels::Shapes::bundle(cursor_position, &mut commands);
            commands.spawn(bundle);
        }
    }
    
    fn on_drag_start(
        trigger: Trigger<Pointer<DragStart>>,
        mut state: ResMut<State>,
    ) {
        if trigger.button != PointerButton::Secondary {
            return;
        }
    
        state.did_drag = true;
    }
    
    fn on_drag_enter(
        trigger: Trigger<Pointer<DragEnter>>,
        droppables: Query<&Droppable>,
        mut commands: Commands,
        mut drop_info: ResMut<DropInfo>,
    ) {
        let Ok(droppable) = droppables.get(trigger.dragged) else {
            return;
        };
    
        drop_info.begin(droppable, trigger, &mut commands);
    }
    
    fn on_drag_over(
        trigger: Trigger<Pointer<DragOver>>,
        droppables: Query<&Droppable>,
        mut commands: Commands,
        mut drop_info: ResMut<DropInfo>,
    ) {
        let Ok(droppable) = droppables.get(trigger.dragged) else {
            return;
        };
    
        drop_info.drag(droppable, trigger, &mut commands);
    }
    
    fn on_drag_drop(
        trigger: Trigger<Pointer<DragDrop>>,
        droppables: Query<&Droppable>,
        mut commands: Commands,
        mut drop_info: ResMut<DropInfo>,
    ) {
        let Ok(droppable) = droppables.get(trigger.dropped) else {
            return;
        };
    
        drop_info.end(droppable, trigger, &mut commands);
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
