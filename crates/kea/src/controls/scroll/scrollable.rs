use bevy::{
    ecs::{
        component::HookContext,
        world::DeferredWorld,
    },
    input::mouse::MouseScrollUnit,
    prelude::*,
};
use crate::{
    constants,
    observers::KeaObservers,
    overrides::KeaNodeOverrides,
};
use super::{
    events::UpdateScrollbars,
    scrollbar::ScrollbarsContainer,
    systems::scrollable,
};

/// Utility component to update a node's scroll position with the mouse wheel.
#[derive(Component)]
#[component(
    on_add = Self::on_add,
)]
#[require(
    KeaObservers = Self::observers(),
    KeaNodeOverrides = Self::overrides(),
    ZIndex(constants::SIZER_Z_INDEX),
    Pickable = Pickable {
        should_block_lower: false,
        is_hoverable: true,
    },
)]
pub struct KeaScrollable;

impl KeaScrollable {
    fn observers() -> KeaObservers {
        KeaObservers::new(vec![
            Observer::new(on_scroll),
            Observer::new(scrollable::on_move),
        ])
    }

    fn overrides() -> KeaNodeOverrides {
        KeaNodeOverrides {
            overflow: Some(Overflow::scroll()),
            ..default()
        }
    }

    fn on_add(
        mut world: DeferredWorld,
        HookContext {
            entity,
            ..
        }: HookContext,
    ) {
        let mut commands = world.commands();
        commands
            .entity(entity)
            .with_child(ScrollbarsContainer::bundle());
    }
}

fn on_scroll(
    trigger: Trigger<Pointer<Scroll>>,
    mut scroll_positions: Query<&mut ScrollPosition>,
    mut commands: Commands,
) {
    let Ok(mut scroll_position) = scroll_positions.get_mut(trigger.target()) else {
        return;
    };

    let scalar = match trigger.event().unit {
        MouseScrollUnit::Line => 12.0,
        MouseScrollUnit::Pixel => 1.0,
    };

    scroll_position.offset_x -= trigger.event().x * scalar;
    scroll_position.offset_y -= trigger.event().y * scalar;

    commands.trigger_targets(UpdateScrollbars, trigger.target());
}
