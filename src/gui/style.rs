use bevy::prelude::*;

///
/// TODO: A proper system where the style can be changed and serialized to disk.
/// For now, we will just use constants.
/// 

pub mod colors {
    use super::*;

    pub const NORMAL: Color = Color::srgb(0.25, 0.25, 0.25);
    pub const HIGHLIGHT: Color = Color::srgb(0.40, 0.40, 0.40);
}
