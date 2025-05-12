use bevy::prelude::*;
use crate::svg;

pub mod buttonex;
pub mod droppable;
pub mod icons;
pub mod menu;
pub mod panels;
pub mod style;

mod sizer;
mod trail;
mod viewport;

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
            .init_resource::<buttonex::State>()
            .add_plugins((
                menu::Plugin,
                panels::Plugin,
            ))
            .add_systems(Startup, setup)
            .add_observer(sizer::Sizer::on_added);

        droppable::build(app);
        icons::build(app);
        trail::build(app);
    }
}

#[derive(Resource)]
pub struct State {
    is_interacting: bool,
    did_drag: bool,
}

impl Default for State {
    fn default() -> Self {
        Self {
            is_interacting: false,
            did_drag: false,
        }
    }
}

impl State {
    pub fn is_interacting(&self) -> bool {
        self.is_interacting
    }
}

//
// Systems
//

fn setup(
    mut commands: Commands,
    mut icons: ResMut<icons::Icons>,
) {
    let mut entity = commands.spawn(viewport::Viewport);
    viewport::Viewport::observe(&mut entity);

    buttonex::ButtonEx::initialize(&mut commands);

    icons.register_callback(commands.register_system(on_icons_loaded));
}

fn on_icons_loaded(
    resources: Res<panels::Resources>,
    mut commands: Commands,
    mut icons: ResMut<icons::Icons>,
    asset_server: Res<AssetServer>,
    svgs: Res<Assets<svg::SvgAsset>>,
) {
    panels::Shapes::create(&mut commands, &mut icons, &asset_server, &svgs, &resources, Vec2::new(50.0, 50.0));
}
