use bevy::{
    asset::AssetPath,
    prelude::*,
};
use crate::assets::svg::SvgLoaderSettings;

///
/// KeaImageNode
///
/// This component will attempt to add a ImageNode component based on the given
/// path.
///
#[derive(Component)]
pub struct KeaImageNode(pub String);

pub(super) fn build(app: &mut App) {
    app.add_observer(on_add);
}

fn parse_size(label: &str) -> Option<Vec2> {
    let Some(size) = label.strip_prefix("image") else {
        return None;
    };

    let tokens: Vec<&str> = size.split("x").collect();
    if tokens.len() != 2 {
        return None;
    }

    let x = match tokens[0].parse::<f32>() {
        Ok(result) => result,
        Err(_) => {
            return None;
        },
    };

    let y = match tokens[1].parse::<f32>() {
        Ok(result) => result,
        Err(_) => {
            return None;
        },
    };

    Some(Vec2::new(x, y))
}

fn on_add(
    trigger: Trigger<OnAdd, KeaImageNode>,
    paths: Query<&KeaImageNode>,
    asset_server: Res<AssetServer>,
    mut commands: Commands,
) {
    let Ok(path) = paths.get(trigger.target()) else {
        return;
    };

    let asset_path = AssetPath::from_static(path.0.clone());

    let extension = match asset_path.get_full_extension() {
        Some(result) => result,
        None => format!(""),
    };

    let label = match asset_path.label() {
        Some(result) => result.to_string(),
        None => format!(""),
    };

    let handle = {
        if extension == "svg" {
            asset_server.load_with_settings(asset_path, move |settings: &mut SvgLoaderSettings| {
                settings.size = parse_size(&label);
            })
        } else {
            asset_server.load(asset_path)
        }
    };

    commands
        .entity(trigger.target())
        .insert(ImageNode::new(handle))
        .remove::<KeaImageNode>();
}
