use bevy::prelude::*;

#[derive(Debug)]
pub enum KeaPropertyData {
    Boolean(bool),
    Integer(i64),
    Decimal(f64),
    Text(String),
    Vector3(Vec3),
}

#[derive(Event)]
pub struct KeaPropertyChanged {
    pub data: KeaPropertyData,
}
