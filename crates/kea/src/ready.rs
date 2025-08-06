use bevy::prelude::*;

/// As of version 0.16, there is no event triggered when all components of an entity have been
/// initialized when an entity is spawned (there is currently a github issue discussing this
/// https://github.com/bevyengine/bevy/issues/18671). This can be problematic when dealing with
/// an entity hierarchy. As a work around until this issue is resolved in the engine, a separate
/// component can be added to request the KeaOnReady event to be triggered for that entity. This
/// event is triggered in the PostUpdate schedule of the current frame to allow for any additional
/// initialization steps to occur.
#[derive(Component, Default)]
pub struct KeaOnReadyComponent;

#[derive(Event)]
pub struct KeaOnReady;

#[derive(Resource)]
struct Pool {
    entities: Vec<Entity>,
}

pub(super) fn build(app: &mut App) {
    app
        .insert_resource(Pool {
            entities: Vec::new(),
        })
        .add_systems(PostUpdate, emit_events)
        .add_observer(on_add);
}

fn on_add(
    trigger: Trigger<OnAdd, KeaOnReadyComponent>,
    mut pool: ResMut<Pool>,
    mut commands: Commands,
) {
    pool.entities.push(trigger.target());

    commands
        .entity(trigger.target())
        .remove::<KeaOnReadyComponent>();
}

fn emit_events(
    mut pool: ResMut<Pool>,
    mut commands: Commands,
) {
    if pool.entities.is_empty() {
        return;
    }

    let entities = core::mem::take(&mut pool.entities);
    commands.trigger_targets(KeaOnReady, entities);
}
