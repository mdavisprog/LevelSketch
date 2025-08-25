use bevy::{
    prelude::*,
    ui::RelativeCursorPosition,
};
use crate::{
    camera,
    level::LevelCommands,
    gui::{
        item::Item,
        trail::{
            DespawnTrail,
            Trail,
        },
        viewport::Viewport,
    },
    tools::selection,
};
use kea::prelude::*;
use super::tools::Tools;

#[derive(Component)]
#[require(
    Node = Self::node(),
    Tools,
)]
pub struct ShapesTools {
    _private: (),
}

impl ShapesTools {
    pub fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            ShapeItem::bundle(ShapeItemType::Cube, "icons/shapes/cuboid.svg#image"),
            ShapeItem::bundle(ShapeItemType::Capsule, "icons/shapes/capsule.svg#image"),
            ShapeItem::bundle(ShapeItemType::Cone, "icons/shapes/cone.svg#image"),
            ShapeItem::bundle(ShapeItemType::ConicalFrustum, "icons/shapes/conical_frustum.svg#image"),
            ShapeItem::bundle(ShapeItemType::Cylinder, "icons/shapes/cylinder.svg#image"),
            ShapeItem::bundle(ShapeItemType::Sphere, "icons/shapes/sphere.svg#image"),
            ShapeItem::bundle(ShapeItemType::Tetrahedron, "icons/shapes/tetrahedron.svg#image"),
            ShapeItem::bundle(ShapeItemType::Torus, "icons/shapes/torus.svg#image"),
        ],
    )}

    // Override the Tools node component.
    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Row,
            row_gap: Val::Px(8.0),
            column_gap: Val::Px(8.0),
            flex_wrap: FlexWrap::Wrap,
            width: Val::Percent(100.0),
            ..default()
        }
    }
}

#[derive(Clone, Copy)]
enum ShapeItemType {
    Capsule,
    Cone,
    ConicalFrustum,
    Cube,
    Cylinder,
    Sphere,
    Tetrahedron,
    Torus,
}

#[derive(Component)]
#[require(RelativeCursorPosition)]
struct ShapeItem {
    shape: ShapeItemType,
}

impl ShapeItem {
    fn bundle(
        shape: ShapeItemType,
        image_path: &str
    ) -> impl Bundle {(
        Self {
            shape,
        },
        Item::bundle((
            KeaImageNode(image_path.to_string()),
            Pickable::IGNORE,
        )),
        KeaObservers::<Self>::new(vec![
            Observer::new(on_shape_item_drag_start),
            Observer::new(on_shape_item_drag_end),
            Observer::new(on_shape_item_drag),
        ]),
    )}
}

#[derive(Resource, Default)]
struct ShapeItemsResource {
    dragging: Option<Entity>,
    last_selection: Vec<Entity>,
    last_point: Vec3,
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

impl ShapeItemsResource {
    fn get_mesh(&self, shape: ShapeItemType) -> Option<Handle<Mesh>> {
        let mesh = match shape {
            ShapeItemType::Capsule => &self.capsule,
            ShapeItemType::Cone => &self.cone,
            ShapeItemType::ConicalFrustum => &self.conical_frustum,
            ShapeItemType::Cube => &self.cube,
            ShapeItemType::Cylinder => &self.cylinder,
            ShapeItemType::Sphere => &self.sphere,
            ShapeItemType::Tetrahedron => &self.tetrahedron,
            ShapeItemType::Torus => &self.torus,
        };

        let Some(mesh) = mesh else {
            return None;
        };

        Some(mesh.clone())
    }

    fn get_material(&self) -> Option<Handle<StandardMaterial>> {
        let Some(material) = &self.material else {
            return None;
        };

        Some(material.clone())
    }
}

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<ShapeItemsResource>()
        .add_systems(Startup, setup)
        .add_observer(on_drag_enter);
}

fn get_point(
    camera: &Query<(&Camera, &GlobalTransform), With<camera::Controller>>,
    screen_position: Vec2,
) -> Option<Vec3> {
    let Ok((camera, camera_transform)) = camera.single() else {
        return None;
    };

    let Ok(ray) = camera.viewport_to_world(camera_transform, screen_position) else {
        return None;
    };

    let origin = camera_transform.translation() + camera_transform.forward() * 10.0;
    let Some(distance) = ray.intersect_plane(origin, InfinitePlane3d::new(Vec3::Y)) else {
        return None;
    };

    Some(ray.get_point(distance))
}

fn setup(
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
    mut resource: ResMut<ShapeItemsResource>,
) {
    resource.cone = Some(meshes.add(Cone::default()));
    resource.cube = Some(meshes.add(Cuboid::default()));
    resource.tetrahedron = Some(meshes.add(Tetrahedron::default()));
    resource.capsule = Some(meshes.add(Capsule3d::default()));
    resource.torus = Some(meshes.add(Torus::default()));
    resource.cylinder = Some(meshes.add(Cylinder::default()));
    resource.conical_frustum = Some(meshes.add(ConicalFrustum::default()));
    resource.sphere = Some(meshes.add(Sphere::default()));
    resource.material = Some(materials.add(Color::srgb(0.5, 0.5, 0.5)));
}

/// TODO: Find a better way to generalize this logic of dragging and dropping any item.
/// This will need to be done when implemented other asset types.

/// Global drag detection. This is used to determine if the ShapeItem node has been dragged into
/// the Viewport node.
fn on_drag_enter(
    trigger: Trigger<Pointer<DragEnter>>,
    viewports: Query<&Viewport>,
    camera: Query<(&Camera, &GlobalTransform), With<camera::Controller>>,
    shape_items: Query<&ShapeItem>,
    mut resource: ResMut<ShapeItemsResource>,
    mut commands: Commands,
    mut selection: ResMut<selection::Selection>,
) {
    if resource.dragging.is_none() {
        return;
    }

    // Check to see if the dragging entity has entered the Viewport node. The Viewport node covers
    // the entire screen and sits below the GUI and on top of the world.
    if !viewports.contains(trigger.target()) {
        return;
    }

    let dragging = resource.dragging.unwrap();
    if dragging != trigger.dragged {
        return;
    }

    // Clear out the current dragging entity from the resource to prevent re-entry of this function.
    resource.dragging = None;

    let Ok(shape_item) = shape_items.get(dragging) else {
        return;
    };

    // The next steps will grab the world position and spawn the associated shape. The spawned
    // shape is then added to the selection resource and will be moved while the shape is being
    // dragged.
    let Some(point) = get_point(&camera, trigger.pointer_location.position) else {
        return;
    };

    let Some(material) = resource.get_material() else {
        return;
    };

    let Some(mesh) = resource.get_mesh(shape_item.shape) else {
        return;
    };

    let spawned_entity = commands.spawn_level((
        Mesh3d(mesh),
        MeshMaterial3d(material),
        Transform::from_translation(point),
    ))
    .id();

    commands.trigger(DespawnTrail);

    resource.last_point = point;
    resource.last_selection = core::mem::take(&mut selection.world);
    selection.world = vec![
        spawned_entity
    ];
}

fn on_shape_item_drag_start(
    trigger: Trigger<Pointer<DragStart>>,
    relative_cursors: Query<&RelativeCursorPosition>,
    nodes: Query<&ComputedNode>,
    mut commands: Commands,
    mut resource: ResMut<ShapeItemsResource>,
) {
    let Ok(relative_cursor) = relative_cursors.get(trigger.target()) else {
        return;
    };

    let Ok(node) = nodes.get(trigger.target()) else {
        return;
    };

    let normalized_position = relative_cursor.normalized.unwrap_or(Vec2::ZERO);

    let offset = Vec2::new(
        node.content_size.x * normalized_position.x,
        node.content_size.y * normalized_position.y,
    );

    resource.dragging = Some(trigger.target());
    commands.spawn(Trail::bundle_with_offset(trigger.target(), -offset));
}

fn on_shape_item_drag_end(
    _: Trigger<Pointer<DragEnd>>,
    mut resource: ResMut<ShapeItemsResource>,
    mut selection: ResMut<selection::Selection>,
) {
    resource.dragging = None;
    selection.world = core::mem::take(&mut resource.last_selection);
}

fn on_shape_item_drag(
    trigger: Trigger<Pointer<Drag>>,
    camera: Query<(&Camera, &GlobalTransform), With<camera::Controller>>,
    mut selection_actions: EventWriter<selection::Action>,
    mut resource: ResMut<ShapeItemsResource>,
) {
    let Some(point) = get_point(&camera, trigger.pointer_location.position) else {
        return;
    };

    let delta = point - resource.last_point;
    resource.last_point = point;
    selection_actions.write(selection::Action::Move(delta));
}
