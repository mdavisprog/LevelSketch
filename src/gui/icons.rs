use bevy::asset::{LoadState, LoadedFolder};
use bevy::ecs::system::SystemId;
use bevy::prelude::*;
use bevy::utils::HashMap;
use crate::svg;

pub fn build(app: &mut App) {
    app
        .init_state::<State>()
        .init_resource::<Icons>()
        .add_systems(Startup, Icons::setup)
        .add_systems(Update, Icons::check_status.run_if(in_state(State::Loading)));
}

#[derive(Resource)]
pub struct Icons {
    items: HashMap<String, Vec<Icon>>,
    handles: Option<Handle<LoadedFolder>>,
    callbacks: Vec<SystemId>,
}

impl Default for Icons {
    fn default() -> Self {
        Self {
            items: HashMap::new(),
            handles: None,
            callbacks: Vec::new(),
        }
    }
}

impl Icons {
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

    pub fn register_callback(&mut self, callback: SystemId) {
        self.callbacks.push(callback);
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

    fn setup(asset_server: Res<AssetServer>, mut icons: ResMut<Self>) {
        icons.handles = Some(asset_server.load_folder("icons"));
    }

    fn check_status(
        asset_server: Res<AssetServer>,
        icons: ResMut<Self>,
        mut commands: Commands,
        mut next_state: ResMut<NextState<State>>,
    ) {
        let Some(handle) = &icons.handles else {
            panic!("Failed to begin loading folder!");
        };

        let Some(state) = asset_server.get_load_state(handle) else {
            panic!("Failed to get loading state!");
        };

        match state {
            LoadState::Loaded => {
                next_state.set(State::Finished);

                for callback in &icons.callbacks {
                    commands.run_system(*callback);
                }

                println!("Finished loading icons!");
            }
            LoadState::Failed(error) => {
                panic!("Failed to load icons folder!\nError: {error}");
            }
            _ => {}
        }
    }
}

struct Icon {
    image: Handle<Image>,
    size: Vec2,
}

#[derive(States, Clone, Copy, Default, Eq, PartialEq, Hash, Debug)]
enum State {
    #[default]
    Loading,
    Finished,
}
