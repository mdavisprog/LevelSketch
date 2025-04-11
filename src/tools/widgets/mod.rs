use bevy::prelude::*;
use super::*;

mod axis;
mod translation;

pub use axis::Axis;
pub use axis::Direction;
pub use translation::Translation;

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
    translation: Entity,
}

impl Default for Widget {
    fn default() -> Self {
        Self {
            root: Entity::PLACEHOLDER,
            translation: Entity::PLACEHOLDER,
        }
    }
}

impl Widget {
    pub fn contains(&self, entity: Entity, children: &Query<&Children>) -> bool {
        if self.root == entity {
            return true;
        }

        if self.translation == entity {
            return true;
        }

        if Self::is_descedent(entity, self.translation, children) {
            return true;
        }

        false
    }

    pub fn get_axis_direction(&self, entity: Entity, children: &Query<&Children>, axes: &Query<&Axis>) -> Option<axis::Direction> {
        let mut direction = axis::Direction::X;
        for child in children.iter_descendants_depth_first(self.translation) {
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

pub fn init(
    commands: &mut Commands,
    meshes: &mut ResMut<Assets<Mesh>>,
    materials: &mut ResMut<Assets<StandardMaterial>>,
) {
    let mut translation = Entity::PLACEHOLDER;
    let mut root = commands.spawn_empty();
    root.with_children(|parent| {
        translation = Translation::new(parent, meshes, materials);
    });

    root.insert(Widget {
        root: root.id(),
        translation
    });
}

pub fn handle_hover(
    axes: Query<&widgets::Axis>,
    meshes: Query<&MeshMaterial3d<StandardMaterial>>,
    children: Query<&Children>,
    widget: Query<&widgets::Widget>,
    mut events: EventReader<Hover>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    let Ok(widget) = widget.get_single() else {
        events.clear();
        return;
    };

    for event in events.read() {
        let (entities, color) = {
            let mut entities = Vec::<Entity>::new();
            let mut color = constants::HOVER_COLOR;

            for child in children.children(widget.translation) {
                if *child == event.target || Widget::is_descedent(event.target, *child, &children) {
                    if let Ok(axis) = axes.get(*child) {
                        color = axis.color;

                        entities.clear();
                        entities.push(*child);
                        for axis_child in children.iter_descendants(*child) {
                            entities.push(axis_child);
                        }
                        break;
                    }
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
