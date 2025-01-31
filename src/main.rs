use bevy::input::keyboard::*;
use bevy::input::mouse::*;
use bevy::prelude::*;
use bevy::window::*;
use bevy::winit::*;
use std::f32::consts::*;
use std::time::Duration;

//
// Types
//

#[derive(Component)]
struct CameraController {
    velocity: Vec3,
    speed: f32,
    max_speed: f32,
    friction: f32,
    is_rotating: bool,
    last_screen_position: Vec2,
    set_last_screen_position: bool,
}

impl Default for CameraController {
    fn default() -> Self {
        Self {
            velocity: Vec3::new(0.0, 0.0, 0.0),
            speed: 0.25,
            max_speed: 20.0,
            friction: 0.1,
            is_rotating: false,
            last_screen_position: Vec2::new(0.0, 0.0),
            set_last_screen_position: false,
        }
    }
}

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
        CameraController {
            velocity: Vec3::ZERO,
            ..default()
        }
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
    mut camera: Query<(&mut Transform, &mut CameraController), With<CameraController>>,
    mut window: Query<&mut Window>,
) {
    let Ok((mut transform, mut camera_controller)) = camera.get_single_mut() else {
        println!("Failed to get camera's transform.");
        return;
    };

    let Ok(mut window) = window.get_single_mut() else {
        return;
    };

    if camera_controller.set_last_screen_position {
        window.set_cursor_position(Some(camera_controller.last_screen_position));
        window.cursor_options.visible = true;
        camera_controller.set_last_screen_position = false;
    }

    if mouse_buttons.just_pressed(MouseButton::Left) {
        camera_controller.is_rotating = true;
        window.cursor_options.visible = false;
        window.cursor_options.grab_mode = CursorGrabMode::Locked;

        camera_controller.last_screen_position = match window.cursor_position() {
            Some(position) => position,
            None => Vec2::new(0.0, 0.0)
        };
    } else if mouse_buttons.just_released(MouseButton::Left) {
        camera_controller.is_rotating = false;
        camera_controller.set_last_screen_position = true;
        window.cursor_options.grab_mode = CursorGrabMode::None;
    }

    let delta_mouse: Vec2 = mouse_move.delta;
    if camera_controller.is_rotating && delta_mouse != Vec2::ZERO {
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
    let direction: Vec3 = if move_forward {
        transform.forward().as_vec3()
    } else if move_backward {
        transform.back().as_vec3()
    } else if move_right {
        transform.right().as_vec3()
    } else if move_left {
        transform.left().as_vec3()
    } else {
        Vec3::ZERO
    };

    if direction != Vec3::ZERO {
        let delta = direction * camera_controller.speed;
        camera_controller.velocity = (camera_controller.velocity + delta).clamp_length(0.0, camera_controller.max_speed);
    } else {
        camera_controller.velocity = camera_controller.velocity * (1.0 - camera_controller.friction);
    }

    transform.translation += camera_controller.velocity * time.delta_secs();
}

fn check_exit(
    mut events: EventWriter<AppExit>,
    keys: Res<ButtonInput<KeyCode>>,
) {
    if keys.just_pressed(KeyCode::Escape) {
        events.send(AppExit::Success);
    }
}

//
// Main
//

fn main() {
    App::new()
        .insert_resource(WinitSettings {
            focused_mode: UpdateMode::Continuous,
            unfocused_mode: UpdateMode::reactive_low_power(Duration::from_secs(2))
        })
        .add_plugins(DefaultPlugins.set(WindowPlugin {
            primary_window: Some(Window {
                title: "Level Sketch".into(),
                ..default()
            }),
            ..default()
        }))
        .add_systems(Startup, setup)
        .add_systems(Update, (update_camera, check_exit))
        .run();
}
