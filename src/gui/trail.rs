/// Module that handles cloning a GUI entity and have it trail some object like the mouse cursor.

use bevy::prelude::*;
use crate::commands::prelude::*;

#[derive(Component)]
#[require(Node)]
pub struct Trail {
    clone: Entity,
}

impl Trail {
    pub fn bundle(clone: Entity) -> impl Bundle {(
        Self {
            clone,
        },
    )}
}

pub(super) fn build(app: &mut App) {
    app
        .add_observer(on_add)
        .add_systems(Update, update);
}

fn on_add(
    trigger: Trigger<OnAdd, Trail>,
    trails: Query<(Entity, &Trail)>,
    mut commands: Commands,
) {
    let Ok((entity, trail)) = trails.get(trigger.target()) else {
        return;
    };

    let cloned = commands.clone_hierarchy(trail.clone).id();
    commands
        .entity(entity)
        .add_child(cloned)
        .insert(Pickable::IGNORE);
}

fn update(
    trails: Query<Entity, With<Trail>>,
    windows: Query<&Window>,
    mouse: Res<ButtonInput<MouseButton>>,
    mut nodes: Query<&mut Node>,
    mut commands: Commands,
) {
    let cursor_position = {
        let mut result = None;

        for window in windows.iter() {
            if let Some(cursor_position) = window.cursor_position() {
                result = Some(cursor_position);
                break;
            }
        }

        result
    };

    let Some(cursor_position) = cursor_position else {
        return;
    };

    for trail in trails.iter() {
        let Ok(mut node) = nodes.get_mut(trail) else {
            continue;
        };

        node.left = Val::Px(cursor_position.x);
        node.top = Val::Px(cursor_position.y);
    }

    if mouse.just_released(MouseButton::Left) {
        for trail in trails.iter() {
            commands.entity(trail).despawn();
        }
    }
}
