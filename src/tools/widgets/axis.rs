use bevy::prelude::*;

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum Direction {
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ,
}

impl std::fmt::Display for Direction {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Direction::X => write!(f, "X"),
            Direction::Y => write!(f, "Y"),
            Direction::Z => write!(f, "Z"),
            Direction::XY => write!(f, "XY"),
            Direction::XZ => write!(f, "XZ"),
            Direction::YZ => write!(f, "YZ"),
        }
    }
}

impl Direction {
    pub fn normal(&self) -> Vec3 {
        match self {
            Direction::X => Vec3::Y,
            Direction::Y => Vec3::Z,
            Direction::Z => Vec3::X,
            Direction::XY => Vec3::Z,
            Direction::XZ => Vec3::Y,
            Direction::YZ => Vec3::X,
        }
    }

    pub fn direction(&self, transform: &Transform) -> Vec3 {
        let result = match self {
            Direction::X => transform.left(),
            Direction::Y => transform.up(),
            Direction::Z => transform.forward(),
            Direction::XY => transform.forward(),
            Direction::XZ => transform.left(),
            Direction::YZ => transform.up(),
        };

        result.as_vec3()
    }
}

#[derive(Component)]
pub struct Axis {
    pub color: Color,
    pub direction: Direction,
}
