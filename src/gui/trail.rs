/// Module that handles cloning a GUI entity and have it trail some object like the mouse cursor.

use bevy::prelude::*;
use crate::commands::prelude::*;

#[derive(Component)]
#[require(
    Node,
    Pickable = Pickable::IGNORE,
)]
pub struct Trail {
    clone: Entity,
    offset: Vec2,
}

impl Trail {
    #[allow(unused)]
    pub fn bundle(clone: Entity) -> impl Bundle {
        Self::bundle_with_offset(clone, Vec2::ZERO)
    }

    pub fn bundle_with_offset(clone: Entity, offset: Vec2) -> impl Bundle {(
        Self {
            clone,
            offset,
        },
        GlobalZIndex(i32::MAX),
    )}
}

#[derive(Event)]
pub struct DespawnTrail;

pub(super) fn build(app: &mut App) {
    app
        .add_observer(on_add)
        .add_observer(on_despawn_trail)
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
        .entity(cloned)
        .insert(Pickable::IGNORE);

    let offset = trail.offset;
    commands
        .entity(entity)
        .add_child(cloned)
        .insert(Pickable::IGNORE)
        .entry::<Node>()
        .and_modify(move |mut node| {
            node.left = Val::Px(offset.x);
            node.top = Val::Px(offset.y);
        });

    // TODO: Should we iterate over all children and mark them as 'Pickable::IGNORE'?
    // One issue is that the relationship isn't set up until the current commands have been executed.
    // For now, just ignoring the root cloned entity.
}

fn on_despawn_trail(
    _: Trigger<DespawnTrail>,
    trails: Query<Entity, With<Trail>>,
    mut commands: Commands,
) {
    for trail in trails {
        commands.entity(trail).despawn();
    }
}

fn update(
    trails: Query<(Entity, &Trail)>,
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

    for (entity, trail) in trails {
        let Ok(mut node) = nodes.get_mut(entity) else {
            continue;
        };

        node.left = Val::Px(cursor_position.x + trail.offset.x);
        node.top = Val::Px(cursor_position.y + trail.offset.y);
    }

    if mouse.just_released(MouseButton::Left) {
        commands.trigger(DespawnTrail);
    }
}
