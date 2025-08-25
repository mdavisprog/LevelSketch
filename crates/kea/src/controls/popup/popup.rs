use bevy::prelude::*;

pub enum KeaPopupPosition {
    At(IVec2),
    AtMouse,
}

pub enum KeaPopupSize {
    Fixed(Vec2),
}

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub(super) struct KeaPopup {
    pub window: Entity,
}

impl KeaPopup {
    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            ..default()
        }
    }
}
