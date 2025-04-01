use bevy::prelude::*;
use bevy::ecs::system::SystemId;

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<DropInfo>();
}

#[derive(Component)]
pub struct Droppable {
    callback: SystemId,
}

impl Droppable {
    pub fn new(callback: SystemId) -> Self {
        Self {
            callback,
        }
    }

    pub fn invoke(&self, commands: &mut Commands) {
        commands.run_system(self.callback);
    }
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum DropState {
    Begin,
    Drag,
    End,
}

#[derive(Resource)]
pub struct DropInfo {
    target: Entity,
    screen_position: Vec2,
    state: DropState,
}

impl Default for DropInfo {
    fn default() -> Self {
        Self {
            target: Entity::PLACEHOLDER,
            screen_position: Vec2::ZERO,
            state: DropState::End,
        }
    }
}

impl DropInfo {
    pub fn begin(
        &mut self,
        droppable: &Droppable,
        trigger: Trigger<Pointer<DragEnter>>,
        commands: &mut Commands,
    ) -> &mut Self {
        if self.state != DropState::End {
            return self;
        }

        self.state = DropState::Begin;
        self.target = trigger.dragged;
        self.screen_position = trigger.pointer_location.position;
        droppable.invoke(commands);
        self
    }

    pub fn drag(
        &mut self,
        droppable: &Droppable,
        trigger: Trigger<Pointer<DragOver>>,
        commands: &mut Commands,
    ) -> &mut Self {
        self.state = DropState::Drag;
        self.target = trigger.dragged;
        self.screen_position = trigger.pointer_location.position;
        droppable.invoke(commands);
        self
    }

    pub fn end(
        &mut self,
        droppable: &Droppable,
        trigger: Trigger<Pointer<DragDrop>>,
        commands: &mut Commands,
    ) -> &mut Self {
        self.state = DropState::End;
        self.target = trigger.dropped;
        self.screen_position = trigger.pointer_location.position;
        droppable.invoke(commands);
        self
    }

    pub fn target(&self) -> Entity {
        self.target
    }

    pub fn screen_position(&self) -> Vec2 {
        self.screen_position
    }

    pub fn state(&self) -> DropState {
        self.state
    }
}
