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
use super::popup::KeaPopup;

#[derive(Resource, Default)]
pub(super) struct WindowPositions {
    pub map: HashMap<Entity, IVec2>,
}

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<WindowPositions>()
        .add_systems(Startup, setup)
        .add_systems(PreUpdate, on_mouse_button)
        .add_systems(Update, (
            on_window_created,
            on_window_moved,
        ));
}

fn setup(mut commands: Commands) {
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
        KeaPopup {
            window,
        },
        UiTargetCamera(camera),
    ));
}

fn on_window_created(
    popups: Query<&ComputedNodeTarget, With<KeaPopup>>,
    cameras: Query<&Camera>,
    mut windows: Query<&mut Window>,
    mut events: EventReader<WindowCreated>,
) {
    if events.is_empty() {
        return;
    }

    let Ok(popup) = popups.single() else {
        return;
    };

    let Ok(camera) = cameras.get(popup.camera().unwrap_or(Entity::PLACEHOLDER)) else {
        return;
    };

    let RenderTarget::Window(window_ref) = camera.target else {
        return;
    };

    let WindowRef::Entity(window_entity) = window_ref else {
        return;
    };

    for event in events.read() {
        if event.window != window_entity {
            continue;
        }

        let Ok(mut window) = windows.get_mut(window_entity) else {
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
    popups: Query<&KeaPopup>,
    mut windows: Query<(Entity, &mut Window)>,
) {
    const BUTTONS: [MouseButton; 3] = [
        MouseButton::Left,
        MouseButton::Right,
        MouseButton::Middle,
    ];

    let Ok(popup) = popups.single() else {
        return;
    };

    let mut mouse_window = Entity::PLACEHOLDER;
    if buttons.any_just_pressed(BUTTONS) {
        for (entity, window) in &windows {
            if window.cursor_position().is_none() {
                continue;
            };

            mouse_window = entity;
            break;
        }

        let Ok((_, mut window)) = windows.get_mut(popup.window) else {
            return;
        };

        if !window.visible {
            return;
        }

        window.visible = popup.window == mouse_window;
    }
}
