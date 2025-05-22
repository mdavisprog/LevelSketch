use bevy::prelude::*;

pub trait ValExt {
    fn to_px_or(&self, or: f32) -> f32;
}

impl ValExt for Val {
    fn to_px_or(&self, or: f32) -> f32 {
        match *self {
            Self::Px(result) => result,
            _ => or,
        }
    }
}
