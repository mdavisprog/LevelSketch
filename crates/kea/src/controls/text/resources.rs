use bevy::prelude::*;

#[derive(Resource)]
pub struct KeaTextInputResource {
    pub(crate) focused: Entity,
}

impl Default for KeaTextInputResource {
    fn default() -> Self {
        Self {
            focused: Entity::PLACEHOLDER,
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
