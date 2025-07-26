use bevy::prelude::*;

#[derive(Event, Debug)]
pub enum KeaExpanderEvent {
    Collapse,
    Expand,
}
