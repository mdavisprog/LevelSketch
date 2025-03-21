use bevy::prelude::*;
use super::*;

#[derive(Event)]
pub struct Open {
    pub position: Vec2,
    pub title: String,
}

pub(super) fn on_open(
    asset_server: Res<AssetServer>,
    svgs: Res<Assets<svg::SvgAsset>>,
    mut events: EventReader<Open>,
    mut commands: Commands,
    mut icons: ResMut<icons::Icons>,
) {
    for event in events.read() {
        Panel::create(
            &mut commands,
            event.position,
            &event.title,
            &asset_server,
            &svgs,
            &mut icons
        );
    }
}
