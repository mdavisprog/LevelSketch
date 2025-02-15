use bevy::prelude::*;

pub mod menu;

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
            .add_plugins(menu::Plugin)
            .add_observer(on_menu_item);
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

fn on_menu_item(
    trigger: Trigger<menu::MenuItemEvent>,
    mut state: ResMut<State>,
) {
    let event = trigger.event();
    state.is_interacting = match *event {
        menu::MenuItemEvent::Leave => false,
        _ => true,
    }
}
