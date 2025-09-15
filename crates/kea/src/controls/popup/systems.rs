use bevy::{
    platform::collections::HashMap,
    prelude::*,
    render::camera::RenderTarget,
    window::{
        WindowCreated,
        WindowLevel,
        WindowRef,
    },
};
use super::popup::{
    KeaPopup,
    KeaPopupState,
    PopupState,
};

#[derive(Resource, Default)]
pub(super) struct WindowPositions {
    pub map: HashMap<Entity, IVec2>,
}

#[derive(Resource)]
pub(super) struct KeaPopupSettings {
    separate_window: bool,
}

impl Default for KeaPopupSettings {
    fn default() -> Self {
        Self {
            separate_window: false,
        }
    }
}

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<WindowPositions>()
        .init_resource::<KeaPopupSettings>()
        .add_systems(Startup, setup)
        .add_systems(PreUpdate, (
            update_state,
            on_mouse_button,
        ).chain())
        .add_systems(Update, (
            on_window_created,
            on_window_moved,
        ));
}

fn setup(
    settings: Res<KeaPopupSettings>,
    mut commands: Commands,
) {
    if settings.separate_window {
        let window = commands.spawn(
            Window {
                decorations: false,
                resizable: false,
                window_level: WindowLevel::AlwaysOnTop,
                skip_taskbar: true,
                // Need the window to be open and visible on the first frame so that all resources
                // are initialized.
                visible: true,
                position: WindowPosition::At(IVec2::ZERO),
                ..default()
            },
        )
        .id();

        let camera = commands.spawn((
            Camera3d::default(),
            Camera {
                target: RenderTarget::Window(WindowRef::Entity(window)),
                ..default()
            }
        ))
        .id();

        commands.spawn((
            KeaPopup::bundle_window(window),
            UiTargetCamera(camera),
        ));
    } else {
        commands.spawn(KeaPopup::bundle());
    }
}

fn on_window_created(
    popups: Query<&KeaPopup>,
    mut windows: Query<&mut Window>,
    mut events: EventReader<WindowCreated>,
) {
    if events.is_empty() {
        return;
    }

    let Ok(popup) = popups.single() else {
        return;
    };

    for event in events.read() {
        if event.window != popup.window {
            continue;
        }

        let Ok(mut window) = windows.get_mut(popup.window) else {
            continue;
        };

        window.visible = false;
    }
}

fn on_window_moved(
    mut events: EventReader<WindowMoved>,
    mut positions: ResMut<WindowPositions>,
) {
    for event in events.read() {
        positions.map.insert(event.window, event.position);
    }
}

fn on_mouse_button(
    buttons: Res<ButtonInput<MouseButton>>,
    nodes: Query<(&ComputedNode, &GlobalTransform)>,
    mut popups: Query<(Entity, &mut KeaPopup)>,
    mut windows: Query<(Entity, &mut Window)>,
    mut visibilities: Query<&mut Visibility>,
) {
    const BUTTONS: [MouseButton; 3] = [
        MouseButton::Left,
        MouseButton::Right,
        MouseButton::Middle,
    ];

    if !buttons.any_just_pressed(BUTTONS) {
        return;
    }

    let Ok((popup_entity, mut popup)) = popups.single_mut() else {
        return;
    };

    let mut mouse_window = Entity::PLACEHOLDER;
    let mut cursor_position = Vec2::ZERO;
    for (entity, window) in &windows {
        let Some(position) = window.cursor_position() else {
            continue;
        };

        mouse_window = entity;
        cursor_position = position;
        break;
    }

    if popup.window != Entity::PLACEHOLDER {
        let Ok((_, mut window)) = windows.get_mut(popup.window) else {
            return;
        };

        if !window.visible {
            return;
        }

        window.visible = popup.window == mouse_window;
    } else {
        let Ok((node, transform)) = nodes.get(popup_entity) else {
            return;
        };

        let Ok(mut visibility) = visibilities.get_mut(popup_entity) else {
            return;
        };

        let bounds = Rect::from_center_size(
            transform.translation().truncate(),
            node.size()
        );

        if bounds.contains(cursor_position) {
            return;
        }

        if popup.state == PopupState::Closing {
            return;
        }

        *visibility = Visibility::Hidden;
        popup.state = PopupState::Closing;
    }
}

fn update_state(
    mut popups: Query<&mut KeaPopup>,
    mut popup_state: ResMut<KeaPopupState>,
) {
    for mut popup in &mut popups {
        match popup.state {
            PopupState::Closing => {
                popup_state.is_open = false;
                popup.state = PopupState::Closed;
            },
            PopupState::Opening => {
                popup_state.is_open = true;
                popup.state = PopupState::Open;
            },
            _ => {},
        }
    }
}
