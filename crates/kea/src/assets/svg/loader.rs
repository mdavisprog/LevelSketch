use bevy::{
    asset::io::Reader,
    asset::LoadContext,
    prelude::*,
};
use super::{
    asset,
    settings::SvgLoaderSettings,
};

#[derive(Default)]
pub(super) struct AssetLoader;

impl bevy::asset::AssetLoader for AssetLoader {
    type Asset = asset::Asset;
    type Settings = SvgLoaderSettings;
    type Error = std::io::Error;

    fn extensions(&self) -> &[&str] {
        &["svg"]
    }

    async fn load(
            &self,
            reader: &mut dyn Reader,
            settings: &Self::Settings,
            load_context: &mut LoadContext<'_>,
    ) -> Result<Self::Asset, Self::Error> {
        let mut bytes = Vec::<u8>::new();

        let _ = reader.read_to_end(&mut bytes).await?;

        let Ok(text) = String::from_utf8(bytes) else {
            return Err(std::io::Error::new(std::io::ErrorKind::InvalidData, "Couldn't read SVG string from bytes!"));
        };

        let asset = asset::Asset(text);

        let (image, label) = {
            if let Some(size) = settings.size {
                (asset.encode_size(size), format!("image{}x{}", size.x, size.y))
            } else {
                (asset.encode(), format!("image"))
            }
        };

        if let Ok(image) = image {
            load_context.add_labeled_asset(label, image);
        }

        Ok(asset)
    }
}
