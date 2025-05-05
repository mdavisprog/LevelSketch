use bevy::prelude::*;
use super::{
    camera,
    gui,
};

pub fn init(app: &mut App) {
    app.add_systems(Startup, setup);
}

fn setup(
    mut commands: Commands,
) {
    gui::menu::create_menu_bar(
        &mut commands,
        vec![
            ("File", Observer::new(file::construct)),
            ("Camera", Observer::new(camera_menu::construct)),
        ],
    );
}

mod file {
    use super::*;

    pub fn construct(
        trigger: Trigger<gui::menu::MenuItemEvent>,
        mut commands: Commands,
    ) {
        if *trigger.event() != gui::menu::MenuItemEvent::Pressed {
            return;
        }

        gui::menu::create_menu(
            &mut commands,
            vec![
                ("Quit", Observer::new(quit)),
            ],
            trigger.target(),
        );
    }

    fn quit(
        trigger: Trigger<gui::menu::MenuItemEvent>,
        mut events: EventWriter<AppExit>,
    ) {
        if *trigger.event() == gui::menu::MenuItemEvent::Pressed {
            events.write(AppExit::Success);
        }
    }
}

mod camera_menu {
    use super::*;

    pub fn construct(
        trigger: Trigger<gui::menu::MenuItemEvent>,
        mut commands: Commands,
    ) {
        if *trigger.event() != gui::menu::MenuItemEvent::Pressed {
            return;
        }

        gui::menu::create_menu(
            &mut commands,
            vec![
                ("Reset", Observer::new(reset)),
            ],
            trigger.target(),
        );
    }

    fn reset(
        trigger: Trigger<gui::menu::MenuItemEvent>,
        mut query: Query<&mut Transform, With<camera::Controller>>,
    ) {
        if *trigger.event() == gui::menu::MenuItemEvent::Pressed {
            for mut camera in query.iter_mut() {
                camera.translation = camera::Controller::DEFAULT_POSITION;
                camera.look_at(Vec3::ZERO, Vec3::Y);
            }
        }
    }
}
