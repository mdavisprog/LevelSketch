use bevy::prelude::*;

#[derive(Resource)]
pub struct KeaTextInputResource {
    pub(crate) focused: Entity,
    pub(super) focus_state: FocusState,
    pub(super) cursor_state: CursorState,
}

impl Default for KeaTextInputResource {
    fn default() -> Self {
        Self {
            focused: Entity::PLACEHOLDER,
            focus_state: FocusState::None,
            cursor_state: CursorState {
                elapsed: 0.0,
                duration: 0.50,
                visible: true,
            },
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

pub(super) struct CursorState {
    pub elapsed: f32,
    pub duration: f32,
    pub visible: bool,
}

impl CursorState {
    pub fn is_complete(&self) -> bool {
        self.elapsed >= self.duration
    }

    pub fn progress(&self) -> f32 {
        self.elapsed / self.duration
    }

    pub fn toggle_visible(&mut self) -> &mut Self {
        self.visible = !self.visible;
        self
    }
}
