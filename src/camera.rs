use bevy::input::keyboard::*;
use bevy::input::mouse::*;
use bevy::prelude::*;
use bevy::window::*;
use std::f32::consts::*;
use super::gui;

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app.add_systems(Update, Controller::update);
    }
}

#[derive(Component)]
#[require(
    Camera3d,
    Transform(Controller::transform),
)]
pub struct Controller {
    pub velocity: Vec3,
    pub speed: f32,
    pub max_speed: f32,
    pub friction: f32,
    pub is_rotating: bool,
    pub last_screen_position: Vec2,
    pub set_last_screen_position: bool,
}

impl Controller {
    pub const DEFAULT_POSITION: Vec3 = Vec3::new(0.0, 4.0, -8.0);

    fn transform() -> Transform {
        Transform::from_translation(Self::DEFAULT_POSITION)
            .looking_at(Vec3::new(0.0, 0.0, 4.0), Vec3::Y)
    }

    fn update(
        time: Res<Time>,
        keys: Res<ButtonInput<KeyCode>>,
        mouse_buttons: Res<ButtonInput<MouseButton>>,
        mouse_move: Res<AccumulatedMouseMotion>,
        gui_state: Res<gui::State>,
        mut camera: Query<(&mut Transform, &mut Controller)>,
        mut window: Query<&mut Window>,
    ) {
        let Ok((mut transform, mut camera_controller)) = camera.get_single_mut() else {
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

        let mouse_button = MouseButton::Right;
    
        if mouse_buttons.just_pressed(mouse_button) && !gui_state.is_interacting() {
            camera_controller.is_rotating = true;
            window.cursor_options.visible = false;
            window.cursor_options.grab_mode = CursorGrabMode::Locked;
    
            camera_controller.last_screen_position = match window.cursor_position() {
                Some(position) => position,
                None => Vec2::new(0.0, 0.0)
            };
        } else if mouse_buttons.just_released(mouse_button) && camera_controller.is_rotating {
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
        let move_up: bool = keys.pressed(KeyCode::KeyQ);
        let move_down: bool = keys.pressed(KeyCode::KeyE);
        let direction: Vec3 = if move_forward {
            transform.forward().as_vec3()
        } else if move_backward {
            transform.back().as_vec3()
        } else if move_right {
            transform.right().as_vec3()
        } else if move_left {
            transform.left().as_vec3()
        } else if move_up {
            transform.up().as_vec3()
        } else if move_down {
            transform.down().as_vec3()
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
}

impl Default for Controller {
    fn default() -> Self {
        Self {
            velocity: Vec3::ZERO,
            speed: 0.25,
            max_speed: 20.0,
            friction: 0.1,
            is_rotating: false,
            last_screen_position: Vec2::ZERO,
            set_last_screen_position: false,
        }
    }
}
