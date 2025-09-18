use bevy::{
    prelude::*,
    window::{
        ExitCondition,
        PrimaryWindow,
    },
};
use bevy::winit::*;
use std::f32::consts::*;
use std::time::Duration;
use kea::prelude::*;

//
// Includes
//

mod camera;
mod commands;
mod entitiy;
mod gui;
mod level;
mod project;
mod settings;
mod shapes;
mod tools;

use level::LevelCommands;

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
            settings::Plugin,
            KeaPlugin::default(),
            DefaultPlugins.set(WindowPlugin {
                primary_window: Some(Window {
                    title: "Level Sketch".into(),
                    position: WindowPosition::At(IVec2::new(100, 100)),
                    ..default()
                }),
                exit_condition: ExitCondition::OnPrimaryClosed,
                ..default()
            }),
            camera::Plugin,
            gui::GUIPlugin,
            level::Plugin,
            project::Plugin,
            tools::Plugin,
        ))
        .add_systems(Startup, setup)
        .add_systems(Update, check_exit);

    app.run();
}

//
// Systems
//

fn setup(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
    mut windows: Query<&mut Window, With<PrimaryWindow>>,
) {
    commands.spawn((
        camera::Controller::default(),
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
        },
    ));

    commands
        .spawn_level((
            Mesh3d(meshes.add(Cuboid::new(2.0, 2.0, 2.0))),
            MeshMaterial3d(materials.add(Color::linear_rgba(0.5, 0.5, 0.5, 1.0))),
            Transform::from_xyz(0.0, 0.0, 0.0),
        ));

    let Ok(mut window) = windows.single_mut() else {
        warn!("Failed to get the primary window component.");
        return;
    };

    window.set_maximized(true);
}

fn check_exit(
    mut events: EventWriter<AppExit>,
    keys: Res<ButtonInput<KeyCode>>,
) {
    if keys.just_pressed(KeyCode::Escape) {
        events.write(AppExit::Success);
    }
}
