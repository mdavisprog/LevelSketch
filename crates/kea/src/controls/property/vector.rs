use bevy::prelude::*;
use crate::{
    observers::KeaObservers,
    style,
};
use super::{
    decimal::KeaPropertyDecimal,
    systems::on_confirm_vector3,
};

#[derive(Component, Debug)]
pub enum KeaPropertyVector3Field {
    X,
    Y,
    Z,
}

struct Vector3FieldX;

struct Vector3FieldY;

struct Vector3FieldZ;

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct KeaPropertyVector3 {
    pub(super) value: Vec3,
}

impl KeaPropertyVector3 {
    pub fn bundle(value: Vec3) -> impl Bundle {(
        Self {
            value,
        },
        children![
            (
                KeaPropertyDecimal::bundle("X", value.x as f64),
                KeaPropertyVector3Field::X,
                KeaObservers::<Vector3FieldX>::new(vec![
                    Observer::new(on_confirm_vector3),
                ]),
            ),
            (
                KeaPropertyDecimal::bundle("Y", value.y as f64),
                KeaPropertyVector3Field::Y,
                KeaObservers::<Vector3FieldY>::new(vec![
                    Observer::new(on_confirm_vector3),
                ]),
            ),
            (
                KeaPropertyDecimal::bundle("Z", value.z as f64),
                KeaPropertyVector3Field::Z,
                KeaObservers::<Vector3FieldZ>::new(vec![
                    Observer::new(on_confirm_vector3),
                ]),
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            column_gap: Val::Px(style::properties::ROW_GAP),
            ..default()
        }
    }
}
