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
    pub const SEPARATOR: Color = Color::srgb(0.45, 0.45, 0.45);
    pub const CURSOR: Color = Color::srgb(0.80, 0.80, 0.80);
}

pub mod properties {
    use super::*;

    pub const BUTTON_PADDING: UiRect = UiRect::px(12.0, 12.0, 4.0, 4.0);
    pub const PADDING: f32 = 4.0;
    pub const SIZER_SIZE: f32 = 5.0;
    pub const BORDER_RADIUS: BorderRadius = BorderRadius::all(Val::Px(3.0));
    pub const FONT_SIZE: f32 = 12.0;
    pub const PANEL_HEADER_FONT_SIZE: f32 = 16.0;
    pub const SEPARATOR_GAP: f32 = 15.0;
    pub const SEPARATOR_SIZE: f32 = 2.0;
    pub const SEPARATOR_PADDING: f32 = 4.0;
    pub const ROW_GAP: f32 = 4.0;
}
