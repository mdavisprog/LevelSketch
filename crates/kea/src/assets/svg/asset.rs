use bevy::{
    asset::RenderAssetUsages,
    image::{
        CompressedImageFormats,
        ImageSampler,
        ImageType,
    },
    prelude::*,
};
use {
    resvg,
    resvg::{
        tiny_skia,
        usvg,
    }
};

#[derive(bevy::prelude::Asset, TypePath)]
pub struct Asset(pub String);

impl Asset {
    pub fn encode(&self) -> Result<Image, String> {
        self.encode_internal(None)
    }

    pub fn encode_size(&self, size: Vec2) -> Result<Image, String> {
        self.encode_internal(Some(size))
    }

    fn encode_internal(&self, size: Option<Vec2>) -> Result<Image, String> {
        let Ok(tree) = usvg::Tree::from_str(&self.0, &usvg::Options::default()) else {
            return Err(format!("Failed to parse SVG document"));
        };

        let final_size = {
            if let Some(size) = size {
                if let Some(desired) = usvg::Size::from_wh(size.x, size.y) {
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

        Ok(image)
    }
}
