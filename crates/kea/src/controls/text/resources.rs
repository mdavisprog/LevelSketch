use bevy::prelude::*;

#[derive(Resource)]
pub struct KeaTextInputResource {
    pub(crate) focused: Entity,
    pub(super) focus_state: FocusState,
}

impl Default for KeaTextInputResource {
    fn default() -> Self {
        Self {
            focused: Entity::PLACEHOLDER,
            focus_state: FocusState::None,
        }
    }
}

impl KeaTextInputResource {
    pub fn focused(&self) -> Entity {
        self.focused
    }

    pub fn has_focused(&self) -> bool {
        self.focused != Entity::PLACEHOLDER
    }
}

pub(super) enum FocusState {
    None,
    Pending(Entity),
}
