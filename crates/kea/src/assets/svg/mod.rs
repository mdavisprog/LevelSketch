use bevy::prelude::*;

mod asset;
mod loader;
mod settings;

pub(crate) use settings::SvgLoaderSettings;

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .init_asset::<asset::Asset>()
            .init_asset_loader::<loader::AssetLoader>();
    }
}
