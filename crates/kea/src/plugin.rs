use bevy::{
    asset::io::AssetSourceBuilder,
    prelude::*
};
use super::{
    assets,
    controls,
    observers,
    overrides,
    position,
};

#[derive(Default)]
pub struct KeaPlugin {
    pub path_to_crate: Option<String>,
}

impl Plugin for KeaPlugin {
    fn build(&self, app: &mut App) {
        let path = match &self.path_to_crate {
            Some(value) => format!("{}/assets/kea", value),
            None => format!("crates/kea/assets/kea"),
        };

        app.register_asset_source(
            "kea",
            AssetSourceBuilder::platform_default(&path, None),
        );
    }

    fn finish(&self, app: &mut App) {
        app.add_plugins((
            assets::Plugin,
            controls::Plugin,
        ));

        observers::build(app);
        overrides::build(app);
        position::build(app);
    }
}
