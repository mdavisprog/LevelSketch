use bevy::prelude::*;
use super::level::Level;

pub(super) fn build(app: &mut App) {
    app.add_systems(Startup, setup);
}

fn setup(mut commands: Commands) {
    commands.spawn(Level::new());
}
