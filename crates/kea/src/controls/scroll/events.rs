use bevy::prelude::*;

/// The target for this event should be the KeaScrollable.
#[derive(Event)]
pub(super) struct UpdateScrollbars;

#[derive(Event, Debug)]
pub(super) enum PlayAnimation {
    Collapse,
    Expand,
}
