use bevy::{
    input::keyboard::*,
    input::mouse::*,
    prelude::*,
    window::*,
};
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
    Transform = Self::transform(),
)]
pub struct Controller {
    pub velocity: Vec3,
    pub speed: f32,
    pub max_speed: f32,
    pub friction: f32,
    pub is_rotating: bool,
    pub last_screen_position: Vec2,
    pub set_last_screen_position: bool,
    rotation_state: RotationState,
    rotation_speed: f32,
}

impl Controller {
    pub const DEFAULT_POSITION: Vec3 = Vec3::new(4.0, 4.0, 8.0);

    fn transform() -> Transform {
        Transform::from_translation(Self::DEFAULT_POSITION)
            .looking_at(Vec3::new(0.0, 1.0, 0.0), Vec3::Y)
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
        let Ok((mut transform, mut camera_controller)) = camera.single_mut() else {
            return;
        };
    
        let Ok(mut window) = window.single_mut() else {
            return;
        };

        let mouse_button = MouseButton::Right;
        match camera_controller.rotation_state {
            RotationState::None => {
                if camera_controller.set_last_screen_position {
                    window.set_cursor_position(Some(camera_controller.last_screen_position));
                    window.cursor_options.visible = true;
                    camera_controller.set_last_screen_position = false;
                }

                if mouse_buttons.just_pressed(mouse_button) && !gui_state.is_interacting() {
                    camera_controller.rotation_state = RotationState::Begin;
                }
            },
            RotationState::Begin => {
                if mouse_buttons.just_released(mouse_button) {
                    camera_controller.rotation_state = RotationState::None;
                }

                if mouse_move.delta != Vec2::ZERO {
                    camera_controller.rotation_state = RotationState::Drag;

                    window.cursor_options.visible = false;
                    window.cursor_options.grab_mode = CursorGrabMode::Locked;
            
                    camera_controller.last_screen_position = match window.cursor_position() {
                        Some(position) => position,
                        None => Vec2::ZERO,
                    };

                    camera_controller.update_rotation(&mut transform, mouse_move.delta, time.delta_secs());
                }
            }
            RotationState::Drag => {
                camera_controller.update_rotation(&mut transform, mouse_move.delta, time.delta_secs());

                if mouse_buttons.just_released(mouse_button) {
                    camera_controller.rotation_state = RotationState::End;
                }
            }
            RotationState::End => {
                camera_controller.set_last_screen_position = true;
                camera_controller.rotation_state = RotationState::None;
                window.cursor_options.grab_mode = CursorGrabMode::None;
            }
        }
    
        let move_forward: bool = keys.pressed(KeyCode::KeyW);
        let move_backward: bool = keys.pressed(KeyCode::KeyS);
        let move_left: bool = keys.pressed(KeyCode::KeyA);
        let move_right: bool = keys.pressed(KeyCode::KeyD);
        let move_up: bool = keys.pressed(KeyCode::KeyE);
        let move_down: bool = keys.pressed(KeyCode::KeyQ);
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

        let delta = direction * camera_controller.speed * time.delta_secs();
        camera_controller.velocity = (camera_controller.velocity + delta).clamp_length(0.0, camera_controller.max_speed);
    
        transform.translation += camera_controller.velocity * time.delta_secs();
        camera_controller.velocity = camera_controller.velocity * (1.0 - camera_controller.friction);
    }

    fn update_rotation(
        &mut self,
        transform: &mut Transform,
        delta: Vec2,
        delta_time: f32,
    ) {
        if self.is_rotating || delta == Vec2::ZERO {
            return;
        }
   
        let delta_yaw = -delta.x * self.rotation_speed * delta_time;
        let delta_pitch = -delta.y * self.rotation_speed * delta_time;

        let (mut yaw, mut pitch, roll) = transform.rotation.to_euler(EulerRot::YXZ);
        yaw = yaw + delta_yaw;

        const PITCH_LIMIT: f32 = FRAC_PI_2 - 0.01;
        pitch = (pitch + delta_pitch).clamp(-PITCH_LIMIT, PITCH_LIMIT);
    
        transform.rotation = Quat::from_euler(EulerRot::YXZ, yaw, pitch, roll);
    }
}

impl Default for Controller {
    fn default() -> Self {
        Self {
            velocity: Vec3::ZERO,
            speed: 100.0,
            max_speed: 20.0,
            friction: 0.1,
            is_rotating: false,
            last_screen_position: Vec2::ZERO,
            set_last_screen_position: false,
            rotation_state: RotationState::None,
            rotation_speed: 0.25,
        }
    }
}

enum RotationState {
    None,
    Begin,
    Drag,
    End,
}
