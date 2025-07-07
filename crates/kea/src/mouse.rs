use bevy::{
    prelude::*,
    winit::cursor::CursorIcon,
};

#[derive(Event, Debug)]
pub(crate) enum ChangeMouseCursor {
    Set {
        window: Entity,
        cursor: CursorIcon,
    },
    Lock {
        window: Entity,
        target: Entity,
    },
    Unlock {
        window: Entity,
        target: Entity,
    },
}

#[derive(Resource)]
struct MouseCursorState {
    lock: Option<Entity>,
}

pub(super) fn build(app: &mut App) {
    app
        .insert_resource(MouseCursorState {
            lock: None,
        })
        .add_observer(on_change_mouse_cursor);
}

fn on_change_mouse_cursor(
    mut trigger: Trigger<ChangeMouseCursor>,
    windows: Query<&Window>,
    mut state: ResMut<MouseCursorState>,
    mut commands: Commands,
) {
    fn is_valid_window(window: Entity, windows: &Query<&Window>,) -> bool {
        if !windows.contains(window) {
            warn!("Given window {window} is invalid!");
            return false;
        }

        true
    }

    let event = trigger.event_mut();

    match event {
        ChangeMouseCursor::Set {
            window,
            cursor,
        } => {
            if !is_valid_window(*window, &windows) {
                return;
            }

            if state.lock.is_none() {
                commands
                    .entity(*window)
                    .insert(core::mem::take(cursor));
            }
        },
        ChangeMouseCursor::Lock {
            window,
            target,
        } => {
            if !is_valid_window(*window, &windows) {
                return;
            }

            if state.lock.is_none() {
                state.lock = Some(*target);
            }
        },
        ChangeMouseCursor::Unlock {
            window,
            target,
        } => {
            if !is_valid_window(*window, &windows) {
                return;
            }

            let Some(lock) = &state.lock else {
                return;
            };

            if *lock != *target {
                return;
            }

            state.lock = None;
        },
    }
}
