use bevy::prelude::*;

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<Selection>()
        .add_event::<Action>()
        .add_systems(Update, handle_actions);
}

#[derive(Resource)]
pub struct Selection {
    pub world: Vec<Entity>,
}

impl Default for Selection {
    fn default() -> Self {
        Self {
            world: Vec::<Entity>::new(),
        }
    }
}

#[derive(Event)]
pub enum Action {
    Move(Vec3),
    Scale(Vec3),
}

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
                }
            }
        }
    }
}
