use bevy::prelude::*;

///
/// TODO: A proper system where the style can be changed and serialized to disk.
/// For now, we will just use constants.
/// 

pub mod colors {
    use super::*;

    pub const BACKGROUND: Color = Color::srgb(0.10, 0.10, 0.10);
    pub const HIGHLIGHT: Color = Color::srgb(0.40, 0.40, 0.40);
    pub const PRESSED: Color = Color::srgb(0.25, 0.25, 0.25);
}

pub mod properties {
    use super::*;

    pub const BUTTON_PADDING: UiRect = UiRect::px(12.0, 12.0, 4.0, 4.0);
    pub const PADDING: f32 = 4.0;
    pub const SIZER_SIZE: f32 = 5.0;
}
