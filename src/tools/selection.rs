use bevy::prelude::*;

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<Selection>()
        .add_event::<Action>()
        .add_event::<SelectionAction>()
        .add_systems(Update, (
            handle_actions,
            handle_selection_actions,
        ));
}

#[derive(Resource)]
pub struct Selection {
    world: Vec<Entity>,
}

impl Default for Selection {
    fn default() -> Self {
        Self {
            world: Vec::<Entity>::new(),
        }
    }
}

impl Selection {
    pub fn world(&self) -> &Vec<Entity> {
        &self.world
    }
}

#[derive(Event)]
pub enum Action {
    Move(Vec3),
    Scale(Vec3),
    Rotation(Vec3),
}

#[derive(Event, Debug)]
pub enum SelectionAction {
    Push(Entity),
    Set(Vec<Entity>),
    Clear,
}

#[derive(Event)]
pub struct SelectionChanged;

fn handle_actions(
    selection: Res<Selection>,
    mut meshes: Query<&mut Transform, With<Mesh3d>>,
    mut events: EventReader<Action>,
) {
    for event in events.read() {
        for item in &selection.world {
            let Ok(mut mesh) = meshes.get_mut(*item) else {
                continue;
            };

            match event {
                Action::Move(delta) => {
                    mesh.translation += delta;
                },
                Action::Scale(delta) => {
                    mesh.scale += delta;
                },
                Action::Rotation(delta) => {
                    mesh.rotate_x(delta.x.to_radians());
                    mesh.rotate_y(delta.y.to_radians());
                    mesh.rotate_z(delta.z.to_radians());
                },
            }
        }
    }
}

fn handle_selection_actions(
    mut events: EventReader<SelectionAction>,
    mut selection: ResMut<Selection>,
    mut commands: Commands,
) {
    for event in events.read() {
        let trigger = match event {
            SelectionAction::Push(entity) => {
                selection.world.push(*entity);
                true
            },
            SelectionAction::Set(entities) => {
                selection.world = entities.clone();
                true
            },
            SelectionAction::Clear => {
                if selection.world.is_empty() {
                    false
                } else {
                    selection.world.clear();
                    true
                }
            },
        };

        if trigger {
            commands.trigger(SelectionChanged);
        }
    }
}
