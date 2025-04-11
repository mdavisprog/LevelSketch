use bevy::core::FrameCount;
use bevy::picking::focus::HoverMap;
use bevy::prelude::*;
use bevy::render::view::RenderLayers;
use super::camera;

mod constants;
pub mod selection;
mod widgets;

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<State>()
            .insert_resource(widgets::Settings::new(true))
            .add_event::<widgets::Hover>()
            .add_event::<widgets::Move>()
            .add_event::<widgets::Scale>()
            .add_plugins(MeshPickingPlugin)
            .add_systems(Startup, setup)
            .add_systems(Update, (
                sync_camera,
                widgets::handle_hover,
                widgets::handle_move,
                widgets::handle_scale,
            ))
            .add_observer(on_over)
            .add_observer(on_out)
            .add_observer(on_pick)
            .add_observer(on_drag_start)
            .add_observer(on_drag_end)
            .add_observer(on_drag);

        selection::build(app);
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
    drag_offset: Vec3,
    is_dragging: bool,
}

impl Default for State {
    fn default() -> Self {
        Self {
            hovered: None,
            drag_offset: Vec3::ZERO,
            is_dragging: false,
        }
    }
}

//
// Utility
//

fn cast_ray(
    camera: &Camera,
    camera_transform: &GlobalTransform,
    screen_position: Vec2,
    widget_position: Vec3,
    normal: Vec3
) -> Vec3 {
    let Ok(ray) = camera.viewport_to_world(camera_transform, screen_position) else {
        return widget_position;
    };

    let Some(distance) = ray.intersect_plane(widget_position, InfinitePlane3d::new(normal)) else {
        return widget_position;
    };

    return ray.get_point(distance);
}

fn is_rotating(camera: &Query<&camera::Controller>) -> bool {
    let Ok(camera) = camera.get_single() else {
        return false;
    };

    return camera.is_rotating;
}

//
// Systems
//

fn setup(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
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

    widgets::init(&mut commands, &mut meshes, &mut materials);
}

fn sync_camera(
    controller: Query<&Transform, With<camera::Controller>>,
    mut camera: Query<&mut Transform, (With<ToolsCamera>, Without<camera::Controller>)>,
) {
    let Ok(controller) = controller.get_single() else {
        return;
    };

    let Ok(mut camera) = camera.get_single_mut() else {
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

    state.hovered = Some(trigger.target);

    if let Ok(widget) = widget.get_single() {
        if widget.contains(trigger.target, &children) {
            events.send(widgets::Hover {
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

    state.hovered = None;

    if let Ok(widget) = widget.get_single() {
        if widget.contains(trigger.target, &children) {
            events.send(widgets::Hover {
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
    mut selection: ResMut<selection::Selection>,
    mut widget: Query<(&mut Visibility, &mut Transform, &widgets::Widget)>,
    mut guard: Local<FrameGuard>,
) {
    if trigger.button != PointerButton::Primary {
        return;
    }

    let Ok((mut visibility, mut transform, widget)) = widget.get_single_mut() else {
        return;
    };

    // Operate on the 'hovered' entity as opposed to the trigger's target.
    // The target may be a UI element which should be ignored and is never
    // set to the 'hovered' property.

    let Some(hovered) = state.hovered else {
        *visibility = Visibility::Hidden;
        selection.world.clear();
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

    selection.world.clear();
    selection.world.push(hovered);

    transform.translation = mesh.translation;
    *visibility = Visibility::Visible;
}

fn on_drag_start(
    trigger: Trigger<Pointer<DragStart>>,
    axes: Query<&widgets::Axis>,
    camera: Query<(&Camera, &GlobalTransform), With<ToolsCamera>>,
    widget: Query<(&Transform, &widgets::Widget)>,
    frame: Res<FrameCount>,
    children: Query<&Children>,
    mut state: ResMut<State>,
    mut guard: Local<FrameGuard>,
    mut events: EventWriter<widgets::Hover>,
) {
    if trigger.button != PointerButton::Primary {
        return;
    }

    let Ok((transform, widget)) = widget.get_single() else {
        return;
    };

    let Some(axis_direction) = widget.get_axis_direction(trigger.target, &children, &axes) else {
        return;
    };

    let Ok((camera, camera_transform)) = camera.get_single() else {
        return;
    };

    let Ok(_) = ScopedFrameGuard::try_new(&mut guard, frame.0) else {
        return;
    };

    state.is_dragging = true;
    state.drag_offset = cast_ray(
        camera,
        camera_transform,
        trigger.pointer_location.position,
        transform.translation,
        axis_direction.normal()) - transform.translation;
    
    events.send(widgets::Hover {
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

    let Ok(widget) = widget.get_single() else {
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
        events.send(widgets::Hover {
            target: hovered,
            hovered: false
        });
    }
}

fn on_drag(
    trigger: Trigger<Pointer<Drag>>,
    axes: Query<&widgets::Axis>,
    camera: Query<(&Camera, &GlobalTransform), With<ToolsCamera>>,
    frame: Res<FrameCount>,
    state: ResMut<State>,
    widget: Query<(&Transform, &widgets::Widget)>,
    children: Query<&Children>,
    mut guard: Local<FrameGuard>,
    mut selection_actions: EventWriter<selection::Action>,
    mut move_widgets: EventWriter<widgets::Move>,
) {
    if trigger.button != PointerButton::Primary {
        return;
    }

    let Ok((transform, widget)) = widget.get_single() else {
        return;
    };

    let Some(axis_direction) = widget.get_axis_direction(trigger.target, &children, &axes) else {
        return;
    };

    let Ok((camera, camera_transform)) = camera.get_single() else {
        return;
    };

    let Ok(_) = ScopedFrameGuard::try_new(&mut guard, frame.0) else {
        return;
    };

    let direction = axis_direction.direction(transform);
    let position = cast_ray(camera, camera_transform, trigger.pointer_location.position, transform.translation, axis_direction.normal()) - state.drag_offset;

    // To move respective to a 2-dimensional plane, just set the translation to the position.
    // This will be useful when these interactable widgets for plane movement are implemented.
    let delta = match axis_direction {
        widgets::Direction::X |
        widgets::Direction::Y |
        widgets::Direction::Z => {
            let v = position - transform.translation;
            v.dot(direction) / direction.dot(direction) * direction
        },
        widgets::Direction::XY |
        widgets::Direction::XZ |
        widgets::Direction::YZ => {
            position - transform.translation
        }
    };

    selection_actions.send(selection::Action::Move(delta));
    move_widgets.send(widgets::Move { delta });
}
