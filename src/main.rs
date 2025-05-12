use bevy::prelude::*;
use bevy::winit::*;
use std::f32::consts::*;
use std::time::Duration;

//
// Includes
//

mod camera;
mod commands;
mod gui;
mod menus;
mod shapes;
mod svg;
mod tools;

//
// Main
//

fn main() {
    let mut app = App::new();
    app
        .insert_resource(WinitSettings {
            focused_mode: UpdateMode::Continuous,
            unfocused_mode: UpdateMode::reactive_low_power(Duration::from_secs(2))
        })
        .add_plugins((
            DefaultPlugins.set(WindowPlugin {
                primary_window: Some(Window {
                    title: "Level Sketch".into(),
                    ..default()
                }),
                ..default()
            }),
            svg::Plugin,
            camera::Plugin,
            gui::GUIPlugin,
            tools::Plugin,
        ))
        .add_systems(Startup, setup)
        .add_systems(Update, check_exit);

    menus::init(&mut app);

    app.run();
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
        camera::Controller::default(),
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

fn check_exit(
    mut events: EventWriter<AppExit>,
    keys: Res<ButtonInput<KeyCode>>,
) {
    if keys.just_pressed(KeyCode::Escape) {
        events.write(AppExit::Success);
    }
}
