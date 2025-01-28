use std::f32::consts::*;
use bevy::{input::{keyboard::*, mouse::*}, prelude::*};

//
// Types
//

#[derive(Component)]
struct CameraController;

//
// Systems
//

fn setup(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>
) {
    commands.spawn((
        Camera3d::default(),
        Transform::from_xyz(4.0, 7.0, -4.0).looking_at(Vec3::new(0.0, 0.0, 0.0), Vec3::Y),
        CameraController
    ));

    commands.spawn((
        Mesh3d(meshes.add(Cuboid::new(2.0, 2.0, 2.0))),
        MeshMaterial3d(materials.add(Color::linear_rgba(0.5, 0.5, 0.5, 1.0))),
        Transform::from_xyz(0.0, 0.0, 0.0)
    ));

    commands.spawn((
        DirectionalLight {
            shadows_enabled: true,
            ..default()
        },
        Transform {
            translation: Vec3::new(0.0, 2.0, 0.0),
            rotation: Quat::from_rotation_x(-PI / 4.0),
            ..default()
        }
    ));
}

fn update_camera(
    time: Res<Time>,
    keys: Res<ButtonInput<KeyCode>>,
    mouse_buttons: Res<ButtonInput<MouseButton>>,
    mouse_move: Res<AccumulatedMouseMotion>,
    mut camera: Query<&mut Transform, With<CameraController>>
) {
    let Ok(mut transform) = camera.get_single_mut() else {
        println!("Failed to get camera's transform.");
        return;
    };

    let should_translate: bool = mouse_buttons.pressed(MouseButton::Left);
    let delta_mouse: Vec2 = mouse_move.delta;
    if should_translate && delta_mouse != Vec2::ZERO {
        const SENSITIVITY: f32 = 0.005;
        
        let delta_yaw: f32 = -delta_mouse.x * SENSITIVITY;
        let delta_pitch: f32 = -delta_mouse.y * SENSITIVITY;
    
        let (mut yaw, mut pitch, roll) = transform.rotation.to_euler(EulerRot::YXZ);
        yaw = yaw + delta_yaw;
    
        const PITCH_LIMIT: f32 = FRAC_PI_2 - 0.01;
        pitch = (pitch + delta_pitch).clamp(-PITCH_LIMIT, PITCH_LIMIT);
    
        transform.rotation = Quat::from_euler(EulerRot::YXZ, yaw, pitch, roll);
    }

    let move_forward: bool = keys.pressed(KeyCode::KeyW);
    let move_backward: bool = keys.pressed(KeyCode::KeyS);
    let move_left: bool = keys.pressed(KeyCode::KeyA);
    let move_right: bool = keys.pressed(KeyCode::KeyD);
    let delta_seconds: f32 = time.delta_secs();
    let mut translation: Vec3 = transform.translation;

    const SPEED: f32 = 20.0;
    if move_forward {
        translation = translation + transform.forward() * SPEED * delta_seconds;
    } else if move_backward {
        translation = translation + transform.back() * SPEED * delta_seconds;
    }

    if move_right {
        translation = translation + transform.right() * SPEED * delta_seconds;
    } else if move_left {
        translation = translation + transform.left() * SPEED * delta_seconds;
    }

    transform.translation = translation;
}

//
// Main
//

fn main() {
    App::new()
        .add_plugins(DefaultPlugins)
        .add_systems(Startup, setup)
        .add_systems(Update, update_camera)
        .run();
}
