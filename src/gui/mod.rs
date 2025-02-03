use bevy::prelude::*;

mod menu;

//
// Public API
//

pub fn close_menus(commands: &mut Commands) {
    menu::close_menus(commands);
}

//
// Types
//

pub struct GUIPlugin;

impl Plugin for GUIPlugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<State>()
            .add_systems(Startup, setup)
            .add_plugins(menu::Plugin)
            .add_observer(menus::on_menu_item);
    }
}

#[derive(Resource)]
pub struct State {
    is_interacting: bool,
}

impl Default for State {
    fn default() -> Self {
        Self {
            is_interacting: false,
        }
    }
}

impl State {
    pub fn is_interacting(&self) -> bool {
        self.is_interacting
    }
}

#[derive(Component)]
#[require(Node(root_node))]
struct Root;

fn root_node() -> Node {
    Node {
        width: Val::Percent(100.0),
        height: Val::Percent(100.0),
        justify_content: JustifyContent::Start,
        flex_direction: FlexDirection::Row,
        column_gap: Val::Px(12.0),
        ..default()
    }
}

//
// Systems
//

fn setup(
    mut commands: Commands
) {
    menu::create_menu_bar(&mut commands, vec![
        ("File", Observer::new(menus::file::construct)),
        ("Help", Observer::new(menus::help::construct)),
    ]);
}

//
// Menus
//

mod menus {
    use super::*;

    pub fn on_menu_item(
        trigger: Trigger<menu::MenuItemEvent>,
        mut state: ResMut<super::State>,
    ) {
        let event = trigger.event();
        state.is_interacting = match *event {
            menu::MenuItemEvent::Leave => false,
            _ => true,
        }
    }

    pub mod file {
        use super::*;
    
        pub fn construct(
            trigger: Trigger<menu::MenuItemEvent>,
            mut commands: Commands,
        ) {
            if *trigger.event() != menu::MenuItemEvent::Pressed {
                return;
            }

            menu::create_menu(
                &mut commands,
                vec![
                    ("Quit", Observer::new(quit)),
                ],
                trigger.entity(),
            );
        }

        fn quit(
            trigger: Trigger<menu::MenuItemEvent>,
            mut events: EventWriter<AppExit>,
        ) {
            if *trigger.event() == menu::MenuItemEvent::Pressed {
                events.send(AppExit::Success);
            }
        }
    }

    pub mod help {
        use super::*;

        pub fn construct(
            trigger: Trigger<menu::MenuItemEvent>,
            mut commands: Commands,
        ) {
            if *trigger.event() != menu::MenuItemEvent::Pressed {
                return;
            }

            menu::create_menu(
                &mut commands,
                vec![
                    ("About", Observer::new(about)),
                ],
                trigger.entity(),
            );
        }

        fn about(
            _: Trigger<menu::MenuItemEvent>
        ) {
        }
    }
}
