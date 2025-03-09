use bevy::asset::RenderAssetUsages;
use bevy::image::{CompressedImageFormats, ImageSampler, ImageType};
use bevy::asset::{AssetLoader, io::Reader, LoadContext};
use bevy::prelude::*;
use {resvg, resvg::{usvg, usvg::Size, tiny_skia}};

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .init_asset::<SvgAsset>()
            .init_asset_loader::<SvgAssetLoader>();
    }
}

pub struct ImageResult {
    pub image: Image,
    pub size: Vec2,
}

#[derive(Asset, TypePath)]
pub struct SvgAsset(pub String);

impl SvgAsset {
    pub fn encode(&self) -> Result<ImageResult, String> {
        self.encode_internal(None)
    }

    pub fn encode_size(&self, size: Vec2) -> Result<ImageResult, String> {
        self.encode_internal(Some(size))
    }

    fn encode_internal(&self, size: Option<Vec2>) -> Result<ImageResult, String> {
        let Ok(tree) = usvg::Tree::from_str(&self.0, &usvg::Options::default()) else {
            return Err(format!("Failed to parse SVG document"));
        };

        let final_size = {
            if let Some(size) = size {
                if let Some(desired) = Size::from_wh(size.x, size.y) {
                    tree.size().expand_to(desired)
                } else {
                    tree.size()
                }
            } else {
                tree.size()
            }
        };

        let final_int_size = final_size.to_int_size();
        let Some(mut pixels) = tiny_skia::Pixmap::new(final_int_size.width(), final_int_size.height()) else {
            return Err(format!("Failed to generate new Pixmap"));
        };

        let scale = Vec2::new(final_size.width() / tree.size().width(), final_size.height() / tree.size().height());
        resvg::render(&tree, tiny_skia::Transform::from_scale(scale.x, scale.y), &mut pixels.as_mut());

        let png_data = match pixels.encode_png() {
            Ok(result) => result,
            Err(error) => {
                return Err(format!("Failed to encode png: {}", error));
            }
        };

        let image = match Image::from_buffer(
            &png_data,
            ImageType::Format(ImageFormat::Png),
            CompressedImageFormats::NONE,
            true,
            ImageSampler::Default,
            RenderAssetUsages::MAIN_WORLD | RenderAssetUsages::RENDER_WORLD,
        ) {
            Ok(result) => result,
            Err(error) => {
                return Err(format!("Failed to generate image: {}", error));
            }
        };

        Ok(ImageResult { image, size: Vec2::new(final_size.width(), final_size.height()) })
    }
}

#[derive(Default)]
struct SvgAssetLoader;

impl AssetLoader for SvgAssetLoader {
    type Asset = SvgAsset;
    type Settings = ();
    type Error = std::io::Error;

    async fn load(
            &self,
            reader: &mut dyn Reader,
            _: &Self::Settings,
            _: &mut LoadContext<'_>,
    ) -> Result<Self::Asset, Self::Error> {
        let mut bytes = Vec::<u8>::new();

        let _ = reader.read_to_end(&mut bytes).await?;

        let Ok(text) = String::from_utf8(bytes) else {
            return Err(std::io::Error::new(std::io::ErrorKind::InvalidData, "Couldn't read SVG string from bytes!"));
        };

        Ok(SvgAsset(text))
    }

    fn extensions(&self) -> &[&str] {
        &["svg"]
    }
}
