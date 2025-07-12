use bevy::prelude::*;
use std::any::TypeId;

#[derive(Event)]
pub struct KeaAnimationComplete {
    pub component_type: TypeId,
    pub field: String,
}
