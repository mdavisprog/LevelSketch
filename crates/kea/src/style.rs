use bevy::prelude::*;

///
/// TODO: A proper system where the style can be changed and serialized to disk.
/// For now, we will just use constants.
/// 

pub mod colors {
    use super::*;

    pub const BACKGROUND: Color = Color::srgb(0.10, 0.10, 0.10);
    pub const BUTTON_BACKGROUND: Color = Color::srgb(0.20, 0.20, 0.20);
    pub const HIGHLIGHT: Color = Color::srgb(0.50, 0.50, 0.50);
    pub const PRESSED: Color = Color::srgb(0.35, 0.35, 0.35);
    pub const TEXT_INPUT_EDIT: Color = Color::srgb(0.25, 0.25, 0.80);
}

pub mod properties {
    use super::*;

    pub const BUTTON_PADDING: UiRect = UiRect::px(12.0, 12.0, 4.0, 4.0);
    pub const PADDING: f32 = 4.0;
    pub const SIZER_SIZE: f32 = 5.0;
    pub const BORDER_RADIUS: BorderRadius = BorderRadius::all(Val::Px(3.0));
    pub const FONT_SIZE: f32 = 12.0;
    pub const CONTROL_HEIGHT: f32 = FONT_SIZE + 12.0;
}
