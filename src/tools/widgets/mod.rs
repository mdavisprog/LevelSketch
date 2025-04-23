use bevy::prelude::*;
use bevy::render::camera::RenderTarget;
use bevy::window::WindowRef;
use super::*;

mod axis;
mod transform;

pub use axis::Axis;
pub use axis::Direction;

const AXIS_OFFSET: f32 = 1.0;
const MIN_SIZE: f32 = 0.5;
const MAX_SIZE: f32 = 20.0;

pub(super) fn build(app: &mut App) {
    app
        .insert_resource(State::new(true))
        .add_event::<Hover>()
        .add_event::<DragStart>()
        .add_event::<Drag>()
        .add_event::<Scale>()
        .add_systems(Startup, setup)
        .add_systems(Update, (
            handle_hover,
            handle_drag_start,
            handle_drag,
            handle_scale,
        ).chain());
}

#[derive(Component)]
#[require(
    Transform,
    Visibility(|| Visibility::Hidden),
)]
pub struct Widget {
    root: Entity,
    drag_offset: Vec3,
}

impl Widget {
    pub fn contains(&self, entity: Entity, children: &Query<&Children>) -> bool {
        if self.root == entity {
            return true;
        }

        if Self::is_descedent(entity, self.root, children) {
            return true;
        }

        false
    }

    fn is_descedent(entity: Entity, parent: Entity, children: &Query<&Children>) -> bool {
        for child in children.iter_descendants(parent) {
            if child == entity {
                return true;
            }
        }

        false
    }
}

#[derive(Resource)]
struct State {
    should_scale: bool,
    drag_offset: Vec3,
    last_position: Vec3,
}

impl State {
    pub fn new(should_scale: bool) -> Self {
        Self {
            should_scale,
            drag_offset: Vec3::ZERO,
            last_position: Vec3::ZERO,
        }
    }
}

#[derive(Event)]
pub struct Hover {
    pub target: Entity,
    pub hovered: bool,
}

pub struct DragData {
    pub target: Entity,
    pub window: Entity,
    pub screen_position: Vec2,
}

#[derive(Event)]
pub struct DragStart(pub DragData);

#[derive(Event)]
pub struct Drag(pub DragData);

/// This event is used to scale the widget based on the distance from the current camera.
/// This is not used to scale the selected entities. This is done through the selection::Action events.
#[derive(Event)]
struct Scale;

#[derive(Component)]
struct WidgetColor(Color);

fn get_heirarchy(
    entity: Entity,
    children: &Query<&Children>,
    parent: &Query<&Parent>,
) -> Vec<Entity> {
    let mut result = Vec::new();

    result.push(entity);

    for child in children.iter_descendants_depth_first(entity) {
        result.push(child);
    }

    for ancestor in parent.iter_ancestors(entity) {
        result.push(ancestor);
    }

    result
}

fn get_axis_direction(
    entity: Entity,
    children: &Query<&Children>,
    parent: &Query<&Parent>,
    axes: &Query<&Axis>,
) -> Option<Direction> {
    let entities = get_heirarchy(entity, children, parent);

    for item in &entities {
        if let Ok(axis) = axes.get(*item) {
            return Some(axis.direction);
        }
    }

    None
}

fn cast_ray(
    window: Entity,
    cameras: &Query<(&Camera, &GlobalTransform), With<ToolsCamera>>,
    screen_position: Vec2,
    translation: Vec3,
    normal: Vec3,
) -> Vec3 {
    let camera_query = {
        let mut camera_result = None;
        let mut transform_result = None;

        for (camera, camera_transform) in cameras.iter() {
            match camera.target {
                RenderTarget::Window(value) => {
                    match value {
                        WindowRef::Primary => {
                            camera_result = Some(camera);
                            transform_result = Some(camera_transform);
                        },
                        WindowRef::Entity(entity) => {
                            if entity == window {
                                camera_result = Some(camera);
                                transform_result = Some(camera_transform);
                            }
                        }
                    }
                }
                _ => {},
            }
        }

        (camera_result, transform_result)
    };

    let (Some(camera), Some(camera_transform)) = camera_query else {
        return translation;
    };

    let Ok(ray) = camera.viewport_to_world(camera_transform, screen_position) else {
        return translation;
    };

    let Some(distance) = ray.intersect_plane(translation, InfinitePlane3d::new(normal)) else {
        return translation;
    };

    return ray.get_point(distance);
}

fn setup(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    let transform = transform::TransformWidget::new(&mut commands, &mut meshes, &mut materials);
    let mut root = commands.spawn_empty();
    let root_id = root.id();

    root.insert(Widget {
        root: root_id,
        drag_offset: Vec3::ZERO,
    })
    .add_child(transform);
}

fn handle_hover(
    colors: Query<&WidgetColor>,
    meshes: Query<&MeshMaterial3d<StandardMaterial>>,
    children: Query<&Children>,
    parent: Query<&Parent>,
    mut events: EventReader<Hover>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    for event in events.read() {
        let (entities, color) = {
            let entities = get_heirarchy(event.target, &children, &parent);
            let mut color = constants::HOVER_COLOR;

            for entity in &entities {
                if let Ok(widget_color) = colors.get(*entity) {
                    color = widget_color.0;
                    break;
                }
            }

            (entities, color)
        };

        for entity in entities {
            let Ok(mesh) = meshes.get(entity) else {
                continue;
            };

            let Some(material) = materials.get_mut(mesh) else {
                continue;
            };

            material.base_color = if event.hovered { constants::HOVER_COLOR } else { color };
        }
    }
}

fn handle_drag_start(
    axes: Query<&Axis>,
    children: Query<&Children>,
    parent: Query<&Parent>,
    cameras: Query<(&Camera, &GlobalTransform), With<ToolsCamera>>,
    mut drag_start_events: EventReader<DragStart>,
    mut widgets: Query<(&mut Widget, &Transform)>,
    mut state: ResMut<State>,
) {
    let Ok((mut widget, widget_transform)) = widgets.get_single_mut() else {
        return;
    };

    for event in drag_start_events.read() {
        let data = &event.0;

        let Some(axis) = get_axis_direction(
            data.target,
            &children,
            &parent,
            &axes,
        ) else {
            continue;
        };

        widget.drag_offset = cast_ray(
            data.window,
            &cameras,
            data.screen_position,
            widget_transform.translation,
            axis.normal(),
        ) - widget_transform.translation;

        state.drag_offset = widget.drag_offset;
        state.last_position = widget_transform.translation;
    }
}

fn handle_drag(
    axes: Query<&Axis>,
    children: Query<&Children>,
    parent: Query<&Parent>,
    cameras: Query<(&Camera, &GlobalTransform), With<ToolsCamera>>,
    transform_types: Query<&transform::TransformType>,
    mut drag_events: EventReader<Drag>,
    mut scale_events: EventWriter<Scale>,
    mut selection_events: EventWriter<selection::Action>,
    mut widgets: Query<&mut Transform, With<Widget>>,
    mut state: ResMut<State>,
) {
    let Ok(mut widget) = widgets.get_single_mut() else {
        return;
    };

    for event in drag_events.read() {
        let data = &event.0;

        let Some(axis) = get_axis_direction(data.target, &children, &parent, &axes) else {
            continue;
        };

        let transform_type = {
            let mut result = None;

            let hierarchy = get_heirarchy(data.target, &children, &parent);
            for entity in &hierarchy {
                if let Ok(transform_type) = transform_types.get(*entity) {
                    result = Some(transform_type);
                    break;
                }
            }

            result
        };

        let Some(transform_type) = transform_type else {
            continue;
        };

        let position = cast_ray(
            data.window,
            &cameras,
            data.screen_position,
            state.last_position,
            axis.normal(),
        ) - state.drag_offset;

        let direction = axis.direction(&widget);

        // To move respective to a 2-dimensional plane, just set the translation to the position.
        let delta = match axis {
            widgets::Direction::X |
            widgets::Direction::Y |
            widgets::Direction::Z => {
                let v = position - state.last_position;
                v.dot(direction) / direction.dot(direction) * direction
            },
            widgets::Direction::XY |
            widgets::Direction::XZ |
            widgets::Direction::YZ => {
                position - state.last_position
            }
        };

        match *transform_type {
            transform::TransformType::Position => {
                widget.translation += delta;
                selection_events.send(selection::Action::Move(delta));
            },
            transform::TransformType::Scale => {
                selection_events.send(selection::Action::Scale(delta));
            }
        }

        state.last_position = position;
    }

    scale_events.send(Scale);
}

fn handle_scale(
    camera: Query<(&Projection, &GlobalTransform), With<ToolsCamera>>,
    state: Res<State>,
    mut events: EventReader<Scale>,
    mut widgets: Query<&mut Transform, With<Widget>>,
) {
    if !state.should_scale {
        return;
    }

    let Ok((projection, transform)) = camera.get_single() else {
        return;
    };

    let fov = match projection {
        Projection::Perspective(perspective) => perspective.fov,
        Projection::Orthographic(_) => std::f32::consts::PI / 4.0,
    };

    for _ in events.read() {
        for mut widget in widgets.iter_mut() {
            let distance = (widget.translation - transform.translation()).length();
            let tan = f32::tan(fov / 2.0);
            // The final term is to clamp it from a max number of units away.
            let scale = distance * (tan * 2.0) * (1.0 / MAX_SIZE);
            widget.scale = Vec3::splat(scale + MIN_SIZE);
        }
    }
}
