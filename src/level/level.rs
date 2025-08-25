use bevy::prelude::*;

#[derive(Component)]
#[require(
    Transform,
    Visibility,
    Name = Name::new("New Level"),
)]
pub struct Level {
    _private: (),
}

impl Level {
    pub(super) fn new() -> Self {
        Self {
            _private: (),
        }
    }
}
