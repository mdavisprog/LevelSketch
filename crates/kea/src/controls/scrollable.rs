use bevy::{
    input::mouse::MouseScrollUnit,
    prelude::*,
};
use crate::observers::KeaObservers;

///
/// KeaScrollable
///
/// Utility component to update a node's scroll position with the mouse wheel.
///
/// TODO: Add support for displaying and interacting with scroll bars.
///
#[derive(Component)]
#[require(
    KeaObservers = Self::observers(),
)]
pub struct KeaScrollable;

impl KeaScrollable {
    fn observers() -> KeaObservers {
        KeaObservers::new(vec![
            Observer::new(on_scroll),
        ])
    }
}

fn on_scroll(
    trigger: Trigger<Pointer<Scroll>>,
    mut scroll_positions: Query<&mut ScrollPosition>,
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
}
