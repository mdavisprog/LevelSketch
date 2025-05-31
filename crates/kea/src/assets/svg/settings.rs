use bevy::prelude::*;
use serde::{
    Deserialize,
    Serialize,
};

#[derive(Serialize, Deserialize, Debug, Clone)]
pub(crate) struct SvgLoaderSettings {
    pub size: Option<Vec2>,
}

impl Default for SvgLoaderSettings {
    fn default() -> Self {
        Self {
            size: None,
        }
    }
}
