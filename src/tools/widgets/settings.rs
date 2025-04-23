use bevy::prelude::*;

#[derive(Resource)]
pub struct Settings {
    pub rotation_rate: f32,
}

impl Default for Settings {
    fn default() -> Self {
        Self {
            rotation_rate: 30.0,
        }
    }
}
