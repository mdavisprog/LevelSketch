use bevy::prelude::*;
use super::*;

mod axis;
mod transform;

pub use axis::Axis;
pub use axis::Direction;

const AXIS_OFFSET: f32 = 1.0;
const MIN_SIZE: f32 = 0.5;
const MAX_SIZE: f32 = 20.0;

#[derive(Component)]
#[require(
    Transform,
    Visibility(|| Visibility::Hidden),
)]
pub struct Widget {
    root: Entity,
}

impl Default for Widget {
    fn default() -> Self {
        Self {
            root: Entity::PLACEHOLDER,
        }
    }
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

    pub fn get_axis_direction(&self, entity: Entity, children: &Query<&Children>, axes: &Query<&Axis>) -> Option<axis::Direction> {
        let mut direction = axis::Direction::X;
        for child in children.iter_descendants_depth_first(self.root) {
            if let Ok(axis) = axes.get(child) {
                direction = axis.direction;
            }

            if entity == child {
                return Some(direction);
            }
        }

        None
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
pub struct Settings {
    should_scale: bool,
}

impl Settings {
    pub fn new(should_scale: bool) -> Self {
        Self {
            should_scale,
        }
    }
}

#[derive(Event)]
pub struct Hover {
    pub target: Entity,
    pub hovered: bool,
}

#[derive(Event)]
pub struct Move {
    pub delta: Vec3,
}

#[derive(Event)]
pub struct Scale;

#[derive(Component)]
struct WidgetColor(Color);

pub fn init(
    commands: &mut Commands,
    meshes: &mut ResMut<Assets<Mesh>>,
    materials: &mut ResMut<Assets<StandardMaterial>>,
) {
    let transform = transform::TransformWidget::new(commands, meshes, materials);
    let mut root = commands.spawn_empty();
    let root_id = root.id();

    root.insert(Widget {
        root: root_id,
    })
    .add_child(transform);
}

pub fn handle_hover(
    colors: Query<&WidgetColor>,
    meshes: Query<&MeshMaterial3d<StandardMaterial>>,
    children: Query<&Children>,
    parent: Query<&Parent>,
    mut events: EventReader<Hover>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    for event in events.read() {
        let (entities, color) = {
            let mut entities = Vec::<Entity>::new();
            let mut color = constants::HOVER_COLOR;

            entities.push(event.target);

            for descendant in children.iter_descendants_depth_first(event.target) {
                entities.push(descendant);
            }

            for ancestor in parent.iter_ancestors(event.target) {
                entities.push(ancestor);
            }

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

pub fn handle_move(
    mut move_events: EventReader<Move>,
    mut scale_events: EventWriter<Scale>,
    mut widgets: Query<&mut Transform, With<Widget>>,
) {
    for event in move_events.read() {
        for mut widget in widgets.iter_mut() {
            widget.translation += event.delta;
        }
    }

    scale_events.send(Scale);
}

pub fn handle_scale(
    camera: Query<(&Projection, &GlobalTransform), With<ToolsCamera>>,
    settings: Res<Settings>,
    mut events: EventReader<Scale>,
    mut widgets: Query<&mut Transform, With<Widget>>,
) {
    if !settings.should_scale {
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
