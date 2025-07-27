use bevy::{
    ecs::{
        component::HookContext,
        world::DeferredWorld,
    },
    prelude::*,
};
use std::marker::PhantomData;

/// Component that holds a list of Observer components. When this
/// component is added, all Observer components are set to watch
/// the owning entity and spawned before the component is removed.
/// 
/// A generic can be supplied to prevent duplicate components from being
/// added to an entity.
#[derive(Component)]
#[component(
    on_add = Self::on_add,
)]
pub struct KeaObservers<T: Send + Sync + 'static> {
    observers: Vec<Observer>,
    _marker: PhantomData<T>,
}

impl<T: Send + Sync + 'static> KeaObservers<T> {
    pub fn new(observers: Vec<Observer>) -> Self {
        Self {
            observers,
            _marker: PhantomData,
        }
    }

    fn on_add(
        mut world: DeferredWorld,
        HookContext {
            entity,
            ..
        }: HookContext,
    ) {
        let observers = if let Some(mut component) = world.get_mut::<Self>(entity) {
            core::mem::take(&mut component.observers)
        } else {
            Vec::<Observer>::new()
        };

        let mut commands = world.commands();

        for mut observer in observers {
            observer.watch_entity(entity);
            commands.spawn(observer);
        }

        commands
            .entity(entity)
            .remove::<Self>();
    }
}
