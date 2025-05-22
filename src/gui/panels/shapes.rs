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
        icons,
        style,
        trail::{Trail, DespawnTrail},
    },
    svg,
    tools::selection,
};
use super::*;

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
    pub fn create(
        commands: &mut Commands,
        icons: &mut ResMut<icons::Icons>,
        asset_server: &Res<AssetServer>,
        svgs: &Res<Assets<svg::SvgAsset>>,
        resources: &Res<Resources>,
        position: Vec2,
    ) {
        let options = PanelOptions {
            title: format!("Shapes"),
            position,
            size: Vec2::new(400.0, 200.0),
            ..default()
        };

        struct ShapeIcon {
            icon: &'static str,
            kind: Shape,
        }

        let shapes = [
            ShapeIcon { icon: "icons/shapes/cuboid.svg", kind: Shape::Cube },
            ShapeIcon { icon: "icons/shapes/cone.svg", kind: Shape::Cone },
            ShapeIcon { icon: "icons/shapes/tetrahedron.svg", kind: Shape::Tetrahedron },
            ShapeIcon { icon: "icons/shapes/capsule.svg", kind: Shape::Capsule },
            ShapeIcon { icon: "icons/shapes/torus.svg", kind: Shape::Torus },
            ShapeIcon { icon: "icons/shapes/cylinder.svg", kind: Shape::Cylinder },
            ShapeIcon { icon: "icons/shapes/conical_frustum.svg", kind: Shape::ConicalFrustum },
            ShapeIcon { icon: "icons/shapes/sphere.svg", kind: Shape::Sphere },
        ];

        let system = commands.register_system(Self::on_drop_item);
        let items = {
            let mut result = Vec::new();

            for shape in shapes {
                let inner = match Self::spawn_inner(shape.icon, icons, asset_server, svgs, commands) {
                    Ok(result) => result,
                    Err(error) => { 
                        println!("{error}");
                        continue;
                    },
                };

                let item = Self::spawn_item(commands, shape.kind, system);
                commands.entity(item).add_child(inner);
                result.push(item);
            }

            result
        };

        let result = Panel::create(commands, &options, resources, Self);
        commands
            .entity(result.components)
            .add_children(&items);
    }

    fn spawn_item(
        commands: &mut Commands,
        shape: Shape,
        system: SystemId,
    ) -> Entity {
        commands
            .spawn((
                Item { shape },
                Droppable::new(system),
            ))
            .observe(Item::on_over)
            .observe(Item::on_out)
            .observe(Item::on_drag_start)
            .id()
    }

    fn spawn_inner(
        icon_name: &str,
        icons: &mut ResMut<icons::Icons>,
        asset_server: &Res<AssetServer>,
        svgs: &Res<Assets<svg::SvgAsset>>,
        commands: &mut Commands,
    ) -> Result<Entity, String> {
        let handle = match icons.get(icon_name, asset_server, svgs) {
            Ok(result) => result,
            Err(error) => return Err(format!("Failed to spawn inner!\n{error}")),
        };

        Ok(commands.spawn(Inner::new(handle)).id())
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
    fn new(image: Handle<Image>) -> impl Bundle {(
        Self,
        ImageNode::new(image),
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
