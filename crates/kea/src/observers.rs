use bevy::prelude::*;
use super::ready::{
    KeaOnReady,
    KeaOnReadyComponent,
};

///
/// KeaObservers
///
/// Component that holds a list of Observer components. When this
/// component is added, all Observer components are set to watch
/// the owning entity and spawned before the component is removed.
///
#[derive(Component)]
pub struct KeaObservers {
    observers: Vec<Observer>,
    observe_parent: bool,
}

impl KeaObservers {
    pub fn new(observers: Vec<Observer>) -> Self {
        Self {
            observers,
            observe_parent: false,
        }
    }

    pub fn new_observe_parent(observers: Vec<Observer>) -> Self {
        Self {
            observers,
            observe_parent: true,
        }
    }
}

#[derive(Component)]
struct LatentDespawn(Entity);

pub(super) fn build(app: &mut App) {
    app.add_observer(on_add);
}

fn on_add(
    trigger: Trigger<OnAdd, KeaObservers>,
    children: Query<&ChildOf>,
    mut observers: Query<&mut KeaObservers>,
    mut commands: Commands,
) {
    let Ok(mut observers) = observers.get_mut(trigger.target()) else {
        return;
    };

    let target = if observers.observe_parent {
        if let Ok(child) = children.get(trigger.target()) {
            child.parent()
        } else {
            trigger.target()
        }
    } else {
        trigger.target()
    };

    for mut observer in core::mem::take(&mut observers.observers) {
        observer.watch_entity(target);
        commands.spawn(observer);
    }

    if observers.observe_parent {
        commands.spawn((
            KeaOnReadyComponent,
            LatentDespawn(trigger.target()),
        ))
        .observe(on_ready);
    } else {
        commands
            .entity(trigger.target())
            .remove::<KeaObservers>();
    }
}

fn on_ready(
    trigger: Trigger<KeaOnReady>,
    latent_despawns: Query<&LatentDespawn>,
    mut commands: Commands,
) {
    let Ok(latent_despawn) = latent_despawns.get(trigger.target()) else {
        return;
    };

    commands.entity(latent_despawn.0).despawn();
    commands.entity(trigger.target()).despawn();
}
