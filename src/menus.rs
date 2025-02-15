use bevy::prelude::*;
use super::gui;

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
            trigger.entity(),
        );
    }

    fn quit(
        trigger: Trigger<gui::menu::MenuItemEvent>,
        mut events: EventWriter<AppExit>,
    ) {
        if *trigger.event() == gui::menu::MenuItemEvent::Pressed {
            events.send(AppExit::Success);
        }
    }
}
