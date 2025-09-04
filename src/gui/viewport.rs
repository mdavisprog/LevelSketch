use bevy::prelude::*;
use kea::prelude::*;
use super::{
    State,
    menus::ToolsMenu,
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
            .observe(Self::on_drag)
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

    fn on_drag(
        trigger: Trigger<Pointer<Drag>>,
        mut state: ResMut<State>,
    ) {
        if trigger.button != PointerButton::Secondary {
            return;
        }

        state.did_drag = true;
    }
    
    fn on_click(
        trigger: Trigger<Pointer<Click>>,
        state: Res<State>,
        mut commands: Commands,
    ) {
        if trigger.button != PointerButton::Secondary {
            return;
        }
    
        if !state.did_drag {
            commands.kea_popup_open(
                ToolsMenu::bundle(),
                KeaPopupPosition::AtMouse,
                KeaPopupSize::Fixed(Vec2::new(150.0, 100.0)),
            );
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
