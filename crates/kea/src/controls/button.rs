use bevy::{
    ecs::system::IntoObserverSystem,
    picking::hover::HoverMap,
    prelude::*,
};
use crate::{
    constants,
    observers::KeaObservers,
    style,
};
use super::{
    image::KeaImageNode,
};

///
/// KeaButton
///
#[derive(Component)]
#[require(
    Node = Self::node(),
    BackgroundColor = style::colors::BACKGROUND,
    ZIndex(constants::BASE_Z_INDEX),
)]
pub struct KeaButton {
    _private: (),
}

impl KeaButton {
    pub fn label_bundle<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>,
        label: &str,
    ) -> impl Bundle {(
        Self::bundle(callback),
        children![(
            Text::new(label),
            TextFont::from_font_size(12.0),
            Pickable::IGNORE,
        )],
    )}

    pub fn image_bundle<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>,
        path: &str,
    ) -> impl Bundle {(
        Self::bundle(callback),
        children![(
            KeaImageNode(path.to_string()),
            Pickable::IGNORE,
        )],
    )}

    fn bundle<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>,
    ) -> impl Bundle {(
        Self {
            _private: (),
        },
        KeaObservers::new(vec![
            Observer::new(callback),
            Observer::new(mouse_over),
            Observer::new(mouse_out),
            Observer::new(mouse_pressed),
            Observer::new(mouse_click),
        ]),
    )}

    fn node() -> Node {
        Node {
            padding: style::properties::BUTTON_PADDING,
            ..default()
        }
    }
}

///
/// KeaButtonClick
///
/// Event that is emitted when the button has been clicked.
///
#[derive(Event)]
pub struct KeaButtonClick;

///
/// State
///
/// Keeps track of information related to all buttons.
///
#[derive(Resource)]
struct State {
    hot_button: Option<Entity>,
}

pub(super) fn build(app: &mut App) {
    app
        .insert_resource(State {
            hot_button: None,
        })
        .add_observer(global_mouse_released);
}

fn mouse_over(
    trigger: Trigger<Pointer<Over>>,
    state: Res<State>,
    mut colors: Query<&mut BackgroundColor, With<KeaButton>>,
) {
    let Ok(mut color) = colors.get_mut(trigger.target) else {
        return;
    };

    if state.hot_button.is_none() {
        *color = style::colors::HIGHLIGHT.into();
    }
}

fn mouse_out(
    trigger: Trigger<Pointer<Out>>,
    state: Res<State>,
    mut colors: Query<&mut BackgroundColor, With<KeaButton>>,
) {
    let Ok(mut color) = colors.get_mut(trigger.target) else {
        return;
    };

    let change_color = match state.hot_button {
        Some(hot) => {
            if hot != trigger.target {
                true
            } else {
                false
            }
        }
        None => true
    };

    if change_color {
        *color = style::colors::BACKGROUND.into();
    }
}

fn mouse_pressed(
    trigger: Trigger<Pointer<Pressed>>,
    mut colors: Query<&mut BackgroundColor, With<KeaButton>>,
    mut state: ResMut<State>,
) {
    let Ok(mut color) = colors.get_mut(trigger.target) else {
        return;
    };

    *color = style::colors::PRESSED.into();
    state.hot_button = Some(trigger.target);
}

fn mouse_click(
    trigger: Trigger<Pointer<Click>>,
    mut commands: Commands,
) {
    commands.trigger_targets(KeaButtonClick, [trigger.target]);
}

///
/// This is observed only globally. Need to detect cases where 
/// the mouse is up, but no longer on the hot button.
///
fn global_mouse_released(
    trigger: Trigger<Pointer<Released>>,
    hovers: Res<HoverMap>,
    mut colors: Query<&mut BackgroundColor, With<KeaButton>>,
    mut state: ResMut<State>,
) {
    match state.hot_button {
        Some(hot) => {
            if let Ok(mut color) = colors.get_mut(hot) {
                if hot == trigger.target {
                    *color = style::colors::HIGHLIGHT.into();
                } else {
                    *color = style::colors::BACKGROUND.into();
                }
            }
        },
        None => {}
    }

    state.hot_button = None;

    let hover_map = &hovers.0[&trigger.pointer_id];
    for (entity, _) in hover_map {
        if let Ok(mut color) = colors.get_mut(*entity) {
            *color = style::colors::HIGHLIGHT.into();
            break;
        }
    }
}
