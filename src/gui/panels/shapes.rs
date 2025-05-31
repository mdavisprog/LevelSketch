use bevy::{
    ecs::system::SystemId,
    prelude::*,
    ui::RelativeCursorPosition,
};
use crate::{
    camera,
    extensions::prelude::*,
    gui::{
        droppable::*,
        style,
        trail::{Trail, DespawnTrail},
    },
    tools::selection,
};
use kea::prelude::*;

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<Handles>()
        .add_systems(Startup, Handles::setup);
}

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct Shapes;

impl Shapes {
    pub fn bundle(
        position: Vec2,
        commands: &mut Commands,
    ) -> impl Bundle {
        let callback = commands.register_system(Self::on_drop_item);
        KeaPanel::bundle(KeaPanelOptions {
            title: format!("Shapes"),
            position,
            size: Vec2::new(400.0, 200.0),
        },
        (
            Self,
            children![
                Item::bundle(Shape::Cube, "icons/shapes/cuboid.svg#image", callback),
                Item::bundle(Shape::Cone, "icons/shapes/cone.svg#image", callback),
                Item::bundle(Shape::Tetrahedron, "icons/shapes/tetrahedron.svg#image", callback),
                Item::bundle(Shape::Capsule, "icons/shapes/capsule.svg#image", callback),
                Item::bundle(Shape::Torus, "icons/shapes/torus.svg#image", callback),
                Item::bundle(Shape::Cylinder, "icons/shapes/cylinder.svg#image", callback),
                Item::bundle(Shape::ConicalFrustum, "icons/shapes/conical_frustum.svg#image", callback),
                Item::bundle(Shape::Sphere, "icons/shapes/sphere.svg#image", callback),
            ]
        ))
    }

    fn on_drop_item(
        drop_info: Res<DropInfo>,
        items: Query<&Item>,
        handles: Res<Handles>,
        camera: Query<(&Camera, Entity), With<camera::Controller>>,
        global_transforms: Query<&GlobalTransform>,
        mut commands: Commands,
        mut selection: ResMut<selection::Selection>,
        mut selection_actions: EventWriter<selection::Action>,
        mut shapes_state: Local<ShapesState>,
    ) {
        let state = drop_info.state();

        if state == DropState::End {
            selection.world = shapes_state.last_selection.clone();
            return;
        }

        let Ok(item) = items.get(drop_info.target()) else {
            return;
        };

        let Ok((camera, camera_entity)) = camera.single() else {
            return;
        };

        let Ok(camera_transform) = global_transforms.get(camera_entity) else {
            return;
        };

        let Ok(ray) = camera.viewport_to_world(camera_transform, drop_info.screen_position()) else {
            return;
        };

        let origin = camera_transform.translation() + camera_transform.forward() * 10.0;
        let Some(distance) = ray.intersect_plane(origin, InfinitePlane3d::new(Vec3::Y)) else {
            return;
        };

        let point = ray.get_point(distance);

        match state {
            DropState::Begin => {
                let material = if let Some(value) = &handles.material {
                    value.clone()
                } else {
                    return;
                };
        
                let mesh = match item.shape {
                    Shape::Cone => &handles.cone,
                    Shape::Cube => &handles.cube,
                    Shape::Tetrahedron => &handles.tetrahedron,
                    Shape::Capsule => &handles.capsule,
                    Shape::Torus => &handles.torus,
                    Shape::Cylinder => &handles.cylinder,
                    Shape::ConicalFrustum => &handles.conical_frustum,
                    Shape::Sphere => &handles.sphere,
                };
        
                let mesh = if let Some(value) = mesh {
                    value.clone()
                } else {
                    return;
                };
        
                let entity = commands.spawn((
                    Mesh3d(mesh),
                    MeshMaterial3d(material),
                    Transform::from_translation(point),
                )).id();

                shapes_state.last_selection = selection.world.clone();
                selection.world.clear();
                selection.world.push(entity);

                commands.trigger(DespawnTrail);
            },
            DropState::Drag => {
                let delta = point - shapes_state.last_point;
                selection_actions.write(selection::Action::Move(delta));
            },
            DropState::End => {},
        }

        shapes_state.last_point = point;
    }

    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Row,
            flex_wrap: FlexWrap::Wrap,
            row_gap: Val::Px(8.0),
            column_gap: Val::Px(8.0),
            ..default()
        }
    }
}

#[derive(Component, Clone)]
#[require(
    Node = Self::node(),
    BackgroundColor = style::colors::NORMAL,
    RelativeCursorPosition,
)]
struct Item {
    shape: Shape,
}

impl Item {
    fn bundle(
        shape: Shape,
        path: &str,
        callback: SystemId,
    ) -> impl Bundle {(
        Self {
            shape
        },
        KeaObservers::new(vec![
            Observer::new(Self::on_over),
            Observer::new(Self::on_out),
            Observer::new(Self::on_drag_start),
        ]),
        Droppable::new(callback),
        children![(
            Inner::bundle(path),
        )]
    )}

    fn on_over(
        trigger: Trigger<Pointer<Over>>,
        mut items: Query<&mut BackgroundColor, With<Item>>,
    ) {
        let Ok(mut item_color) = items.get_mut(trigger.target) else {
            return;
        };

        *item_color = style::colors::HIGHLIGHT.into();
    }

    fn on_out(
        trigger: Trigger<Pointer<Out>>,
        mut items: Query<&mut BackgroundColor, With<Item>>,
    ) {
        let Ok(mut item_color) = items.get_mut(trigger.target) else {
            return;
        };

        *item_color = style::colors::NORMAL.into();
    }

    fn on_drag_start(
        trigger: Trigger<Pointer<DragStart>>,
        nodes: Query<&Node>,
        relative_positions: Query<&RelativeCursorPosition>,
        mut commands: Commands,
    ) {
        let offset: Vec2 = if let Ok(relative_position) = relative_positions.get(trigger.target()) {
            if let Some(result) = relative_position.normalized {
                if let Ok(node) = nodes.get(trigger.target()) {
                    let width = node.width.to_px_or(0.0);
                    let height = node.height.to_px_or(0.0);

                    Vec2::new(
                        width * result.x,
                        height * result.y,
                    )
                } else {
                    Vec2::ZERO
                }
            } else {
                Vec2::ZERO
            }
        } else {
            Vec2::ZERO
        };

        commands.spawn(Trail::bundle_with_offset(trigger.target(), -offset));
    }

    fn node() -> Node {
        Node {
            width: Val::Px(60.0),
            height: Val::Px(60.0),
            align_items: AlignItems::Center,
            justify_content: JustifyContent::Center,
            ..default()
        }
    }
}

#[derive(Component, Clone)]
#[require(
    Node,
    Pickable = Pickable::IGNORE,
)]
struct Inner;

impl Inner {
    fn bundle(path: &str) -> impl Bundle {(
        Self,
        KeaImageNode(path.to_string()),
    )}
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum Shape {
    Cube,
    Cone,
    Tetrahedron,
    Capsule,
    Torus,
    Cylinder,
    ConicalFrustum,
    Sphere,
}

#[derive(Resource, Default)]
struct Handles {
    cone: Option<Handle<Mesh>>,
    cube: Option<Handle<Mesh>>,
    tetrahedron: Option<Handle<Mesh>>,
    capsule: Option<Handle<Mesh>>,
    torus: Option<Handle<Mesh>>,
    cylinder: Option<Handle<Mesh>>,
    conical_frustum: Option<Handle<Mesh>>,
    sphere: Option<Handle<Mesh>>,
    material: Option<Handle<StandardMaterial>>,
}

impl Handles {
    fn setup(
        mut meshes: ResMut<Assets<Mesh>>,
        mut materials: ResMut<Assets<StandardMaterial>>,
        mut handles: ResMut<Handles>,
    ) {
        handles.cone = Some(meshes.add(Cone::default()));
        handles.cube = Some(meshes.add(Cuboid::default()));
        handles.tetrahedron = Some(meshes.add(Tetrahedron::default()));
        handles.capsule = Some(meshes.add(Capsule3d::default()));
        handles.torus = Some(meshes.add(Torus::default()));
        handles.cylinder = Some(meshes.add(Cylinder::default()));
        handles.conical_frustum = Some(meshes.add(ConicalFrustum::default()));
        handles.sphere = Some(meshes.add(Sphere::default()));
        handles.material = Some(materials.add(Color::srgb(0.5, 0.5, 0.5)));
    }
}

struct ShapesState {
    last_point: Vec3,
    last_selection: Vec<Entity>,
}

impl Default for ShapesState {
    fn default() -> Self {
        Self {
            last_point: Vec3::ZERO,
            last_selection: Vec::new(),
        }
    }
}
