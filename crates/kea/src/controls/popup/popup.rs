use bevy::prelude::*;
use crate::constants;

pub enum KeaPopupPosition {
    At(IVec2),
    AtMouse,
}

pub enum KeaPopupSize {
    Fixed(Vec2),
}

#[derive(Resource, Default)]
pub struct KeaPopupState {
    pub(super) is_open: bool,
}

impl KeaPopupState {
    pub fn is_open(&self) -> bool {
        self.is_open
    }
}

#[derive(Component)]
#[require(
    ZIndex(constants::POPUP_Z_INDEX),
)]
pub(super) struct KeaPopup {
    pub(super) window: Entity,
    pub(super) state: PopupState,
}

impl KeaPopup {
    pub fn bundle_window(window: Entity) -> impl Bundle {(
        Self {
            window,
            state: PopupState::Closed,
        },
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            ..default()
        },
    )}

    pub fn bundle() -> impl Bundle {(
        Self {
            window: Entity::PLACEHOLDER,
            state: PopupState::Closed,
        },
        Node::default(),
        Visibility::Hidden,
    )}
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub(super) enum PopupState {
    Closed,
    Closing,
    Open,
    Opening,
}
