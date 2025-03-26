use bevy::prelude::*;
use super::*;

#[derive(Event)]
pub struct Open {
    pub title: String,
    pub position: Vec2,
    pub size: Vec2,
}

impl Default for Open {
    fn default() -> Self {
        Self {
            title: format!(""),
            position: Vec2::ZERO,
            size: Vec2::new(100.0, 200.0),
        }
    }
}

pub(super) fn on_open(
    resources: Res<Resources>,
    mut events: EventReader<Open>,
    mut commands: Commands,
) {
    for event in events.read() {
        Panel::create(
            &mut commands,
            &event,
            &resources,
        );
    }
}
