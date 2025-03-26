use bevy::prelude::*;
use bevy::utils::HashMap;
use crate::gui::icons;
use crate::svg;

pub fn initialize(app: &mut App) {
    app
        .init_resource::<Resources>()
        .init_resource::<LoadingIcons>()
        .add_systems(Update, Resources::load.run_if(resource_exists::<LoadingIcons>));
}

#[derive(Resource)]
pub struct Resources {
    icons: HashMap<String, Handle<Image>>,
}

impl Default for Resources {
    fn default() -> Self {
        Self {
            icons: HashMap::new(),
        }
    }
}

impl Resources {
    pub fn get_icon(&self, name: &str) -> Option<Handle<Image>> {
        if let Some(result) = self.icons.get(name) {
            Some(result.clone())
        } else {
            None
        }
    }

    fn load(
        asset_server: Res<AssetServer>,
        svgs: Res<Assets<svg::SvgAsset>>,
        mut icons: ResMut<icons::Icons>,
        mut resources: ResMut<Self>,
        mut commands: Commands,
    ) {
        let paths = [
            "icons/close.svg"
        ];

        let mut loaded = 0;
        for path in paths {
            if !resources.icons.contains_key(path) {
                if let Ok(icon) = icons.get_size(path, Vec2::new(12.0, 12.0), &asset_server, &svgs) {
                    resources.icons.insert(path.into(), icon);
                    loaded += 1;
                }
            } else {
                loaded += 1;
            }
        }

        if paths.len() == loaded {
            commands.remove_resource::<LoadingIcons>();
        }
    }
}

#[derive(Resource, Default)]
struct LoadingIcons;
