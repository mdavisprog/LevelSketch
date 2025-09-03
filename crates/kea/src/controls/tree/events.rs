use bevy::prelude::*;

#[derive(Event)]
pub struct KeaTreeHover {
    pub hovered: bool,
}

#[derive(Event)]
pub struct KeaTreeClick;
