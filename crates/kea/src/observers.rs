use bevy::prelude::*;

///
/// KeaObservers
///
/// Component that holds a list of Observer components. When this
/// component is added, all Observer components are set to watch
/// the owning entity and spawned before the component is removed.
///
#[derive(Component)]
pub struct KeaObservers(Vec<Observer>);

impl KeaObservers {
    pub fn new(observers: Vec<Observer>) -> Self {
        Self(observers)
    }
}

pub(super) fn build(app: &mut App) {
    app.add_observer(on_add);
}

fn on_add(
    trigger: Trigger<OnAdd, KeaObservers>,
    mut observers: Query<&mut KeaObservers>,
    mut commands: Commands,
) {
    let Ok(mut observers) = observers.get_mut(trigger.target()) else {
        return;
    };

    for mut observer in core::mem::take(&mut observers.0) {
        observer.watch_entity(trigger.target());
        commands.spawn(observer);
    }

    commands
        .entity(trigger.target())
        .remove::<KeaObservers>();
}
