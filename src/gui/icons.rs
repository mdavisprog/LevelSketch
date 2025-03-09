use bevy::asset::LoadedFolder;
use bevy::prelude::*;
use bevy::utils::HashMap;
use crate::svg;

#[derive(Resource)]
pub struct Icons {
    items: HashMap<String, Vec<Icon>>,
    handles: Option<Handle<LoadedFolder>>,
}

impl Default for Icons {
    fn default() -> Self {
        Self {
            items: HashMap::new(),
            handles: None,
        }
    }
}

impl Icons {
    pub fn initialize(&mut self, asset_server: &Res<AssetServer>) {
        self.handles = Some(asset_server.load_folder("icons"));
    }

    // TODO: Remove once used.
    #[allow(unused)]
    pub fn get(
        &mut self,
        name: &str,
        asset_server: &Res<AssetServer>,
        svgs: &Res<Assets<svg::SvgAsset>>,
    ) -> Result<Handle<Image>, String> {
        self.get_internal(name, None, asset_server, svgs)
    }

    pub fn get_size(
        &mut self,
        name: &str,
        size: Vec2,
        asset_server: &Res<AssetServer>,
        svgs: &Res<Assets<svg::SvgAsset>>,
    ) -> Result<Handle<Image>, String> {
        self.get_internal(name, Some(size), asset_server, svgs)
    }

    fn get_internal(
        &mut self,
        name: &str,
        size: Option<Vec2>,
        asset_server: &Res<AssetServer>,
        svgs: &Res<Assets<svg::SvgAsset>>,
    ) -> Result<Handle<Image>, String> {
        if let Some(handle) = self.get_handle(name, size) {
            return Ok(handle);
        }

        let handle = asset_server.load(name);

        let Some(svg) = svgs.get(&handle) else {
            return Err(format!("Failed to find SVG asset for icon '{name}'"));
        };

        let image_result = {
            if let Some(size) = size {
                svg.encode_size(size)
            } else {
                svg.encode()
            }
        };

        let image = match image_result {
            Ok(result) => result,
            Err(error) => return Err(error),
        };

        if !self.items.contains_key(name) {
            self.items.insert(name.to_string(), Vec::new());
        }

        let image_handle = asset_server.add(image.image);
        if let Some(value) = self.items.get_mut(name) {
            value.push(Icon { image: image_handle.clone(), size: image.size });
        };

        Ok(image_handle)
    }

    fn get_handle(&self, name: &str, size: Option<Vec2>) -> Option<Handle<Image>> {
        if let Some(icons) = self.items.get(name) {
            if let Some(size) = size {
                for icon in icons {
                    if icon.size == size {
                        return Some(icon.image.clone())
                    }
                }
            } else {
                if let Some(icon) = icons.first() {
                    return Some(icon.image.clone());
                }
            }

            None
        } else {
            None
        }
    }
}

struct Icon {
    image: Handle<Image>,
    size: Vec2,
}
