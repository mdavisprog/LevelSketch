use bevy::prelude::*;
use crate::camera;
use kea::prelude::*;
use super::tools::Tools;

#[derive(Component)]
#[require(Tools)]
pub struct CameraTools {
    _private: (),
}

impl CameraTools {
    pub fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            (
                KeaButton::label_bundle("Reset", |
                    _: Trigger<KeaButtonClick>,
                    mut cameras: Query<&mut Transform, With<camera::Controller>>,
                | {
                    let Ok(mut transform) = cameras.single_mut() else {
                        return;
                    };

                    transform.translation = camera::Controller::DEFAULT_POSITION;
                    transform.look_at(Vec3::ZERO, Vec3::Y);
                }),
            ),
            (
                KeaPropertyDecimal::bundle("Speed", 0.0),
                KeaOnReadyComponent,
                KeaObservers::<properties::Speed>::new(vec![
                    Observer::new(on_speed_changed),
                    Observer::new(on_speed_ready),
                ]),
            ),
        ],
    )}
}

mod properties {
    pub struct Speed;
}

fn on_speed_changed(
    trigger: Trigger<KeaPropertyChanged>,
    mut cameras: Query<&mut camera::Controller>,
) {
    let Ok(mut camera) = cameras.single_mut() else {
        return;
    };

    let KeaPropertyData::Decimal(speed) = trigger.event().data else {
        return;
    };

    camera.speed = speed as f32;
    camera.max_speed = camera.speed;
}

fn on_speed_ready(
    trigger: Trigger<KeaOnReady>,
    cameras: Query<&camera::Controller>,
    mut commands: Commands,
) {
    let Ok(camera) = cameras.single() else {
        return;
    };

    commands.kea_property_set_value(trigger.target(), camera.speed.to_string());
}
