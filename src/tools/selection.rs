use bevy::prelude::*;

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<Selection>()
        .add_event::<Move>()
        .add_systems(Update, handle_move);
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
pub struct Move {
    pub delta: Vec3,
}

fn handle_move(
    selection: Res<Selection>,
    mut meshes: Query<&mut Transform, With<Mesh3d>>,
    mut events: EventReader<Move>,
) {
    for event in events.read() {
        for item in &selection.world {
            let Ok(mut mesh) = meshes.get_mut(*item) else {
                continue;
            };
    
            mesh.translation += event.delta;
        }
    }
}
