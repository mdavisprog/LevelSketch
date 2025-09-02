use bevy::{
    diagnostic::FrameCount,
    picking::hover::HoverMap,
    prelude::*,
    render::{
        view::RenderLayers,
        camera::NormalizedRenderTarget,
    },
};
use super::camera;

mod constants;
pub mod selection;
mod widgets;

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<State>()
            .add_plugins(MeshPickingPlugin)
            .add_systems(Startup, setup)
            .add_systems(Update, sync_camera)
            .add_observer(on_over)
            .add_observer(on_out)
            .add_observer(on_pick)
            .add_observer(on_drag_start)
            .add_observer(on_drag_end)
            .add_observer(on_drag);

        selection::build(app);
        widgets::build(app);
    }
}

#[derive(Component)]
struct ToolsCamera;

#[derive(Default)]
struct FrameGuard {
    last_frame: u32,
}

struct ScopedFrameGuard<'a> {
    guard: &'a mut FrameGuard,
    current_frame: u32,
}

impl<'a> Drop for ScopedFrameGuard<'a> {
    fn drop(&mut self) {
        self.guard.last_frame = self.current_frame;
    }
}

impl<'a> ScopedFrameGuard<'a> {
    fn try_new(guard: &'a mut FrameGuard, current_frame: u32) -> Result<Self, &'static str> {
        if guard.last_frame == current_frame {
            return Err("Given frame is the Guard's current frame!");
        }

        Ok(Self {
            guard,
            current_frame,
        })
    }
}

#[derive(Resource)]
struct State {
    hovered: Option<Entity>,
    is_dragging: bool,
}

impl Default for State {
    fn default() -> Self {
        Self {
            hovered: None,
            is_dragging: false,
        }
    }
}

//
// Utility
//

fn is_rotating(camera: &Query<&camera::Controller>) -> bool {
    let Ok(camera) = camera.single() else {
        return false;
    };

    return camera.is_rotating;
}

//
// Systems
//

fn setup(
    mut commands: Commands,
) {
    commands.spawn((
        ToolsCamera,
        Camera3d::default(),
        Camera {
            clear_color: ClearColorConfig::None,
            order: 1,
            ..default()
        },
        Transform::from_translation(camera::Controller::DEFAULT_POSITION)
            .looking_at(Vec3::ZERO, Vec3::Y),
        RenderLayers::layer(constants::RENDER_LAYER),
    ));
}

fn sync_camera(
    controller: Query<&Transform, With<camera::Controller>>,
    mut camera: Query<&mut Transform, (With<ToolsCamera>, Without<camera::Controller>)>,
) {
    let Ok(controller) = controller.single() else {
        return;
    };

    let Ok(mut camera) = camera.single_mut() else {
        return;
    };

    camera.translation = controller.translation;
    camera.rotation = controller.rotation;
    camera.scale = controller.scale;
}

fn on_over(
    trigger: Trigger<Pointer<Over>>,
    camera: Query<&camera::Controller>,
    meshes: Query<&MeshMaterial3d<StandardMaterial>>,
    widget: Query<&widgets::Widget>,
    children: Query<&Children>,
    mut state: ResMut<State>,
    mut events: EventWriter<widgets::Hover>,
) {
    if is_rotating(&camera) || state.is_dragging {
        return;
    }

    if !meshes.contains(trigger.target) {
        return;
    }

    if let Some(hovered) = state.hovered {
        if hovered == trigger.target {
            return;
        }
    }

    state.hovered = Some(trigger.target);

    if let Ok(widget) = widget.single() {
        if widget.contains(trigger.target, &children) {
            events.write(widgets::Hover {
                target: trigger.target,
                hovered: true
            });
        }
    };
}

fn on_out(
    trigger: Trigger<Pointer<Out>>,
    meshes: Query<&MeshMaterial3d<StandardMaterial>>,
    widget: Query<&widgets::Widget>,
    children: Query<&Children>,
    mut state: ResMut<State>,
    mut events: EventWriter<widgets::Hover>,
) {
    if !meshes.contains(trigger.target) || state.is_dragging {
        return;
    }

    if state.hovered.is_none() {
        return;
    }

    if let Some(hovered) = state.hovered {
        if hovered != trigger.target {
            return;
        }
    }

    state.hovered = None;

    if let Ok(widget) = widget.single() {
        if widget.contains(trigger.target, &children) {
            events.write(widgets::Hover {
                target: trigger.target,
                hovered: false
            });
        }
    }
}

fn on_pick(
    trigger: Trigger<Pointer<Click>>,
    meshes: Query<&Transform, (With<Mesh3d>, Without<widgets::Widget>)>,
    frame: Res<FrameCount>,
    state: Res<State>,
    children: Query<&Children>,
    mut widget: Query<(&mut Visibility, &mut Transform, &widgets::Widget)>,
    mut guard: Local<FrameGuard>,
    mut selection_actions: EventWriter<selection::SelectionAction>,
) {
    if trigger.button != PointerButton::Primary {
        return;
    }

    let Ok((mut visibility, mut transform, widget)) = widget.single_mut() else {
        return;
    };

    // Operate on the 'hovered' entity as opposed to the trigger's target.
    // The target may be a UI element which should be ignored and is never
    // set to the 'hovered' property.

    let Some(hovered) = state.hovered else {
        *visibility = Visibility::Hidden;
        selection_actions.write(selection::SelectionAction::Clear);
        return;
    };

    let Ok(mesh) = meshes.get(hovered) else {
        return;
    };

    if widget.contains(hovered, &children) {
        return;
    }

    let Ok(_) = ScopedFrameGuard::try_new(&mut guard, frame.0) else {
        return;
    };

    selection_actions.write(selection::SelectionAction::Clear);
    selection_actions.write(selection::SelectionAction::Push(hovered));

    transform.translation = mesh.translation;
    *visibility = Visibility::Visible;
}

fn on_drag_start(
    trigger: Trigger<Pointer<DragStart>>,
    widgets: Query<&widgets::Widget>,
    frame: Res<FrameCount>,
    children: Query<&Children>,
    mut state: ResMut<State>,
    mut guard: Local<FrameGuard>,
    mut drag_start_events: EventWriter<widgets::DragStart>,
    mut hover_events: EventWriter<widgets::Hover>,
) {
    if trigger.button != PointerButton::Primary {
        return;
    }

    let Ok(widget) = widgets.single() else {
        return;
    };

    if !widget.contains(trigger.target, &children) {
        return;
    }

    let Ok(_) = ScopedFrameGuard::try_new(&mut guard, frame.0) else {
        return;
    };

    state.is_dragging = true;

    let window = match trigger.pointer_location.target {
        NormalizedRenderTarget::Window(value) => value.entity(),
        _ => Entity::PLACEHOLDER,
    };

    drag_start_events.write(widgets::DragStart(widgets::DragData {
        target: trigger.target,
        window,
        screen_position: trigger.pointer_location.position,
    }));

    hover_events.write(widgets::Hover {
        target: trigger.target,
        hovered: true
    });
}

fn on_drag_end(
    trigger: Trigger<Pointer<DragEnd>>,
    widget: Query<&widgets::Widget>,
    children: Query<&Children>,
    hovers: Res<HoverMap>,
    mut state: ResMut<State>,
    mut events: EventWriter<widgets::Hover>,
) {
    if trigger.button != PointerButton::Primary {
        return;
    }

    state.is_dragging = false;

    let Some(hovered) = state.hovered else {
        return;
    };

    let Ok(widget) = widget.single() else {
        return;
    };

    let mut is_hovered = false;
    let hover_map = &hovers.0[&trigger.pointer_id];
    for (entity, _) in hover_map {
        if hovered == *entity {
            is_hovered = true;
            break;
        }
    }

    if widget.contains(hovered, &children) && !is_hovered {
        events.write(widgets::Hover {
            target: hovered,
            hovered: false
        });
    }
}

fn on_drag(
    trigger: Trigger<Pointer<Drag>>,
    frame: Res<FrameCount>,
    widget: Query<&widgets::Widget>,
    children: Query<&Children>,
    mut guard: Local<FrameGuard>,
    mut drag_widgets: EventWriter<widgets::Drag>,
) {
    if trigger.button != PointerButton::Primary {
        return;
    }

    let Ok(widget) = widget.single() else {
        return;
    };

    if !widget.contains(trigger.target, &children) {
        return;
    }

    let Ok(_) = ScopedFrameGuard::try_new(&mut guard, frame.0) else {
        return;
    };

    let window = match trigger.pointer_location.target {
        NormalizedRenderTarget::Window(value) => value.entity(),
        _ => Entity::PLACEHOLDER,
    };

    drag_widgets.write(widgets::Drag( widgets::DragData {
        target: trigger.target,
        window,
        screen_position: trigger.pointer_location.position,
    }));
}
