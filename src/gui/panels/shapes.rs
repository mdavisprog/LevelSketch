use bevy::ecs::system::SystemId;
use bevy::prelude::*;
use crate::camera;
use crate::gui::style;
use crate::gui::droppable::{DropInfo, Droppable};
use super::*;

pub(super) fn build(app: &mut App) {
    app
        .init_resource::<Handles>()
        .add_systems(Startup, Handles::setup);
}

#[derive(Component)]
#[require(
    Node(Self::node)
)]
pub struct Shapes;

impl Shapes {
    pub fn create(
        commands: &mut Commands,
        resources: &Res<Resources>,
        position: Vec2,
    ) {
        let options = PanelOptions {
            title: format!("Shapes"),
            position,
            size: Vec2::new(400.0, 200.0),
            ..default()
        };

        let system = commands.register_system(Self::on_drop_item);

        let result = Panel::create(commands, &options, resources, Self);
        commands
            .entity(result.components)
            .with_children(|parent| {
                Self::spawn_item(parent, Shape::Cube, system);
                Self::spawn_item(parent, Shape::Cone, system);
            });
    }

    fn spawn_item(
        parent: &mut ChildBuilder,
        shape: Shape,
        system: SystemId,
    ) {
        parent
            .spawn((
                Item { shape },
                Droppable::new(system),
            ))
            .with_children(|parent| {
                parent.spawn(Inner);
            })
            .observe(Item::on_over)
            .observe(Item::on_out);
    }

    fn on_drop_item(
        drop_info: Res<DropInfo>,
        items: Query<&Item>,
        handles: Res<Handles>,
        camera: Query<(&Camera, &GlobalTransform), With<camera::Controller>>,
        mut commands: Commands,
    ) {
        let Ok(item) = items.get(drop_info.target) else {
            return;
        };

        let Ok((camera, camera_transform)) = camera.get_single() else {
            return;
        };

        let Ok(ray) = camera.viewport_to_world(camera_transform, drop_info.screen_position) else {
            return;
        };

        let origin = camera_transform.forward() * 10.0;
        let Some(distance) = ray.intersect_plane(origin, InfinitePlane3d::new(Vec3::Y)) else {
            return;
        };

        let point = ray.get_point(distance);

        let material = if let Some(value) = &handles.material {
            value.clone()
        } else {
            return;
        };

        let mesh = match item.shape {
            Shape::Cone => {
                &handles.cone
            },
            Shape::Cube => {
                &handles.cube
            },
        };

        let mesh = if let Some(value) = mesh {
            value.clone()
        } else {
            return;
        };

        commands.spawn((
            Mesh3d(mesh),
            MeshMaterial3d(material),
            Transform::from_translation(point),
        ));
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

#[derive(Component)]
#[require(
    Node(Self::node),
    BackgroundColor(|| style::colors::NORMAL),
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

#[derive(Component)]
#[require(
    Node(Self::node),
    BackgroundColor(|| Color::srgb(0.3, 0.6, 0.3)),
    PickingBehavior(|| PickingBehavior::IGNORE),
)]
struct Inner;

impl Inner {
    fn node() -> Node {
        Node {
            width: Val::Px(40.0),
            height: Val::Px(40.0),
            ..default()
        }
    }
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum Shape {
    Cube,
    Cone,
}

#[derive(Resource, Default)]
struct Handles {
    cone: Option<Handle<Mesh>>,
    cube: Option<Handle<Mesh>>,
    material: Option<Handle<StandardMaterial>>,
}

impl Handles {
    fn setup(
        mut meshes: ResMut<Assets<Mesh>>,
        mut materials: ResMut<Assets<StandardMaterial>>,
        mut handles: ResMut<Handles>,
    ) {
        handles.cone = Some(meshes.add(Cone::new(1.0, 1.0)));
        handles.cube = Some(meshes.add(Cuboid::new(1.0, 1.0, 1.0)));
        handles.material = Some(materials.add(Color::srgb(0.5, 0.5, 0.5)));
    }
}
