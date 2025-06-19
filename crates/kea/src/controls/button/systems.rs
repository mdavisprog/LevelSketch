use bevy::{
    picking::hover::HoverMap,
    prelude::*
};
use crate::style;
use super::{
    component::KeaButton,
    events::KeaButtonClick,
    State
};

pub(super) fn mouse_over(
    trigger: Trigger<Pointer<Over>>,
    state: Res<State>,
    mut colors: Query<&mut BackgroundColor, With<KeaButton>>,
) {
    let Ok(mut color) = colors.get_mut(trigger.target) else {
        return;
    };

    if state.hot_button.is_none() {
        *color = style::colors::HIGHLIGHT.into();
    }
}

pub(super) fn mouse_out(
    trigger: Trigger<Pointer<Out>>,
    state: Res<State>,
    mut colors: Query<&mut BackgroundColor, With<KeaButton>>,
) {
    let Ok(mut color) = colors.get_mut(trigger.target) else {
        return;
    };

    let change_color = match state.hot_button {
        Some(hot) => {
            if hot != trigger.target {
                true
            } else {
                false
            }
        }
        None => true
    };

    if change_color {
        *color = style::colors::BUTTON_BACKGROUND.into();
    }
}

pub(super) fn mouse_pressed(
    trigger: Trigger<Pointer<Pressed>>,
    mut colors: Query<&mut BackgroundColor, With<KeaButton>>,
    mut state: ResMut<State>,
) {
    let Ok(mut color) = colors.get_mut(trigger.target) else {
        return;
    };

    *color = style::colors::PRESSED.into();
    state.hot_button = Some(trigger.target);
}

pub(super) fn mouse_click(
    trigger: Trigger<Pointer<Click>>,
    mut commands: Commands,
) {
    commands.trigger_targets(KeaButtonClick, [trigger.target]);
}

///
/// This is observed only globally. Need to detect cases where 
/// the mouse is up, but no longer on the hot button.
///
pub(super) fn global_mouse_released(
    trigger: Trigger<Pointer<Released>>,
    hovers: Res<HoverMap>,
    mut colors: Query<&mut BackgroundColor, With<KeaButton>>,
    mut state: ResMut<State>,
) {
    match state.hot_button {
        Some(hot) => {
            if let Ok(mut color) = colors.get_mut(hot) {
                if hot == trigger.target {
                    *color = style::colors::HIGHLIGHT.into();
                } else {
                    *color = style::colors::BACKGROUND.into();
                }
            }
        },
        None => {}
    }

    state.hot_button = None;

    let hover_map = &hovers.0[&trigger.pointer_id];
    for (entity, _) in hover_map {
        if let Ok(mut color) = colors.get_mut(*entity) {
            *color = style::colors::HIGHLIGHT.into();
            break;
        }
    }
}
