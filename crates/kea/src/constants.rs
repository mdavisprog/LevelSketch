///
/// Module containing useful constants that can be used across the crate.
///

const BASE_Z_INDEX_LAYER: i32 = 20;
pub const BASE_Z_INDEX: i32 = i32::MAX - BASE_Z_INDEX_LAYER;
pub const PANEL_FOCUSED_Z_INDEX: i32 = BASE_Z_INDEX + 1;
pub const SIZER_Z_INDEX: i32 = PANEL_FOCUSED_Z_INDEX + 1;
