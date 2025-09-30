use bevy::prelude::*;

mod app;
mod resource;

pub use resource::SettingsResource;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<resource::SettingsResource>()
            .add_systems(Startup, on_startup)
            .add_systems(Last, on_exit);
    }
}

fn on_startup(
    mut settings: ResMut<SettingsResource>,
) {
    if let Err(error) = settings.load() {
        warn!("Failed to load settings. Using defaults.");
        warn!("{error:?}");
    }
}

fn on_exit(
    settings: Res<SettingsResource>,
    mut events: EventReader<AppExit>,
) {
    for _ in events.read() {
        if let Err(error) = settings.save() {
            error!("Failed to save settings: {error:?}");
        }
    }
}
