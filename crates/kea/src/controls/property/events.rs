use bevy::prelude::*;

#[derive(Debug)]
pub enum KeaPropertyData {
    Text(String),
    Decimal(f64),
    Vector3(Vec3),
}

#[derive(Event)]
pub struct KeaPropertyChanged {
    pub data: KeaPropertyData,
}
