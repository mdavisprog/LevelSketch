use bevy::prelude::*;
use bevy::render::view::RenderLayers;
use crate::shapes;
use crate::tools::constants;
use super::*;

#[derive(Component)]
pub(super) enum TransformType {
    Position,
    Scale,
    Rotation(Direction),
}

impl std::fmt::Display for TransformType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Self::Position => write!(f, "Position"),
            Self::Scale => write!(f, "Scale"),
            Self::Rotation(dir) => write!(f, "Rotation: {dir}"),
        }
    }
}

#[derive(Component)]
#[require(Transform, Visibility)]
pub(super) struct TransformWidget;

impl TransformWidget {
    pub fn new(
        commands: &mut Commands,
        meshes: &mut ResMut<Assets<Mesh>>,
        materials: &mut ResMut<Assets<StandardMaterial>>,
    ) -> Entity {
        let cylinder = Cylinder::new(0.05, 0.5);
        let cylinder_height = cylinder.lateral_area();
        let cylinder_base = cylinder.base_area();

        let cone = Cone::new(0.1, 0.25);
        let cone_height = cone.slant_height();

        let plane_half_size = 0.15;
        let plane_size = plane_half_size * 2.0;
        let plane_offset = plane_size * 1.5;
        let plane = Plane3d::new(Vec3::Y, Vec2::splat(plane_half_size));

        let cylinder_mesh = meshes.add(cylinder);
        let cone_mesh = meshes.add(cone);
        let plane_mesh = meshes.add(plane);
        let layer = RenderLayers::layer(constants::RENDER_LAYER);

        let scale_controls = Self::spawn_scales(
            commands,
            meshes,
            materials,
            cylinder_height + cone_height + 1.25,
        );

        let rotation_controls = Self::spawn_rotations(
            commands,
            meshes,
            materials,
            0.75,
        );

        let mut entity = commands.spawn(Self);
        entity.with_children(|parent| {
            // X-Axis
            parent.spawn((
                layer.clone(),
                Mesh3d(cylinder_mesh.clone()),
                MeshMaterial3d(materials.add(Self::standard_material(constants::X_AXIS_COLOR))),
                WidgetColor(constants::X_AXIS_COLOR),
                axis::Axis {
                    direction: axis::Direction::X,
                },
                TransformType::Position,
                Transform::from_rotation(
                    Quat::from_rotation_z(90.0_f32.to_radians()))
                    .with_translation(Vec3::new(AXIS_OFFSET, 0.0, 0.0)
                ),
            ))
            .with_children(|parent| {
                parent.spawn((
                    layer.clone(),
                    Mesh3d(cone_mesh.clone()),
                    MeshMaterial3d(materials.add(Self::standard_material(constants::X_AXIS_COLOR))),
                    Transform::from_rotation(
                        Quat::from_rotation_z(180_f32.to_radians()))
                        .with_translation(Vec3::new(0.0, -cylinder_height - cone_height * 0.5, 0.0)
                    ),
                ));
            });

            // Y-Axis
            parent.spawn((
                layer.clone(),
                Mesh3d(cylinder_mesh.clone()),
                MeshMaterial3d(materials.add(Self::standard_material(constants::Y_AXIS_COLOR))),
                WidgetColor(constants::Y_AXIS_COLOR),
                axis::Axis {
                    direction: axis::Direction::Y,
                },
                TransformType::Position,
                Transform::from_xyz(0.0, AXIS_OFFSET, 0.0),
            ))
            .with_children(|parent| {
                parent.spawn((
                    layer.clone(),
                    Mesh3d(cone_mesh.clone()),
                    MeshMaterial3d(materials.add(Self::standard_material(constants::Y_AXIS_COLOR))),
                    Transform::from_translation(Vec3::new(0.0, cylinder_height + cone_height * 0.5, 0.0)),
                ));
            });

            // Z-Axis
            parent.spawn((
                layer.clone(),
                Mesh3d(cylinder_mesh.clone()),
                MeshMaterial3d(materials.add(Self::standard_material(constants::Z_AXIS_COLOR))),
                WidgetColor(constants::Z_AXIS_COLOR),
                axis::Axis {
                    direction: axis::Direction::Z,
                },
                TransformType::Position,
                Transform::from_rotation(
                    Quat::from_rotation_x(90.0_f32.to_radians()))
                    .with_translation(Vec3::new(0.0, 0.0, AXIS_OFFSET)
                ),
            ))
            .with_children(|parent| {
                parent.spawn((
                    layer.clone(),
                    Mesh3d(cone_mesh.clone()),
                    MeshMaterial3d(materials.add(Self::standard_material(constants::Z_AXIS_COLOR))),
                    Transform::from_xyz(0.0, cylinder_height + cone_height * 0.5, 0.0),
                ));
            });

            // XY plane
            parent.spawn((
                layer.clone(),
                Mesh3d(plane_mesh.clone()),
                MeshMaterial3d(materials.add(Self::standard_material_no_cull(constants::Z_AXIS_COLOR))),
                WidgetColor(constants::Z_AXIS_COLOR),
                axis::Axis {
                    direction: axis::Direction::XY,
                },
                TransformType::Position,
                Transform::from_xyz(
                    plane_offset, plane_offset, cylinder_base * -0.5)
                    .with_rotation(Quat::from_rotation_x(90.0_f32.to_radians())
                ),
                RayCastBackfaces,
            ));

            // XZ plane
            parent.spawn((
                layer.clone(),
                Mesh3d(plane_mesh.clone()),
                MeshMaterial3d(materials.add(Self::standard_material_no_cull(constants::Y_AXIS_COLOR))),
                WidgetColor(constants::Y_AXIS_COLOR),
                axis::Axis {
                    direction: axis::Direction::XZ,
                },
                TransformType::Position,
                Transform::from_xyz(plane_offset, cylinder_base * -0.5, plane_offset),
                RayCastBackfaces,
            ));

            // YZ plane
            parent.spawn((
                layer.clone(),
                Mesh3d(plane_mesh.clone()),
                MeshMaterial3d(materials.add(Self::standard_material_no_cull(constants::X_AXIS_COLOR))),
                WidgetColor(constants::X_AXIS_COLOR),
                axis::Axis {
                    direction: axis::Direction::YZ,
                },
                TransformType::Position,
                Transform::from_xyz(
                    cylinder_base * -0.5, plane_offset, plane_offset)
                    .with_rotation(Quat::from_rotation_z(-90.0_f32.to_radians())
                ),
                RayCastBackfaces,
            ));
        });

        entity
            .add_children(&scale_controls)
            .add_children(&rotation_controls);

        entity.id()
    }

    fn spawn_scales(
        commands: &mut Commands,
        meshes: &mut ResMut<Assets<Mesh>>,
        materials: &mut ResMut<Assets<StandardMaterial>>,
        offset: f32,
    ) -> [Entity; 3] {
        let mut result = [Entity::PLACEHOLDER; 3];

        result[0] = Self::spawn_scale(
            commands,
            meshes,
            materials,
            Vec3::new(offset, 0.0, 0.0),
            constants::X_AXIS_COLOR,
            Direction::X,
        );

        result[1] = Self::spawn_scale(
            commands,
            meshes,
            materials,
            Vec3::new(0.0, offset, 0.0),
            constants::Y_AXIS_COLOR,
            Direction::Y,
        );

        result[2] = Self::spawn_scale(
            commands,
            meshes,
            materials,
            Vec3::new(0.0, 0.0, offset),
            constants::Z_AXIS_COLOR,
            Direction::Z,
        );

        result
    }

    fn spawn_scale(
        commands: &mut Commands,
        meshes: &mut ResMut<Assets<Mesh>>,
        materials: &mut ResMut<Assets<StandardMaterial>>,
        offset: Vec3,
        color: Color,
        direction: axis::Direction,
    ) -> Entity {
        let cuboid = Cuboid::from_length(0.15);
        let cuboid_mesh = meshes.add(cuboid);

        commands.spawn((
            RenderLayers::layer(constants::RENDER_LAYER),
            Mesh3d(cuboid_mesh),
            MeshMaterial3d(materials.add(Self::standard_material(color))),
            WidgetColor(color),
            TransformType::Scale,
            Transform::from_translation(offset),
            axis::Axis {
                direction,
            }
        )).id()
    }

    fn spawn_rotations(
        commands: &mut Commands,
        meshes: &mut ResMut<Assets<Mesh>>,
        materials: &mut ResMut<Assets<StandardMaterial>>,
        offset: f32,
    ) -> [Entity; 3] {
        let mut result = [Entity::PLACEHOLDER; 3];

        let arc = shapes::AnnulusSector::new(25.0_f32.to_radians(), 35.0_f32.to_radians(), 60.0_f32.to_radians());
        let mesh = meshes.add(arc);

        result[0] = commands.spawn(Self::rotation_bundle(
            mesh.clone(),
            materials.add(Self::standard_material_no_cull(constants::Y_AXIS_COLOR)),
            constants::Y_AXIS_COLOR,
            Quat::from_euler(EulerRot::XYZ, 90.0_f32.to_radians(), 0.0, -45.0_f32.to_radians()),
            Vec3::new(offset, 0.0, offset),
            Direction::Y,
            Direction::X,
        )).id();

        result[1] = commands.spawn(Self::rotation_bundle(
            mesh.clone(),
            materials.add(Self::standard_material_no_cull(constants::Z_AXIS_COLOR)),
            constants::Z_AXIS_COLOR,
            Quat::from_euler(EulerRot::XYZ, 45.0_f32.to_radians(), 90.0_f32.to_radians(), 0.0),
            Vec3::new(0.0, offset, offset),
            Direction::X,
            Direction::Y,
        )).id();

        result[2] = commands.spawn(Self::rotation_bundle(
            mesh.clone(),
            materials.add(Self::standard_material_no_cull(constants::X_AXIS_COLOR)),
            constants::X_AXIS_COLOR,
            Quat::from_euler(EulerRot::XYZ, 0.0, 0.0, -45.0_f32.to_radians()),
            Vec3::new(offset, offset, 0.0),
            Direction::Z,
            Direction::Y,
        )).id();

        result
    }

    fn rotation_bundle(
        mesh: Handle<Mesh>,
        material: Handle<StandardMaterial>,
        color: Color,
        rotation: Quat,
        translation: Vec3,
        rotation_direction: Direction,
        translation_direction: Direction,
    ) -> impl Bundle {(
        RenderLayers::layer(constants::RENDER_LAYER),
        Mesh3d(mesh),
        MeshMaterial3d(material),
        WidgetColor(color),
        TransformType::Rotation(rotation_direction),
        Transform::from_rotation(rotation).with_translation(translation),
        axis::Axis {
            direction: translation_direction,
        },
        RayCastBackfaces,
    )}

    fn standard_material(color: Color) -> StandardMaterial {
        StandardMaterial {
            base_color: color,
            alpha_mode: AlphaMode::Opaque,
            unlit: true,
            ..default()
        }
    }

    fn standard_material_no_cull(color: Color) -> StandardMaterial {
        let mut result = Self::standard_material(color);
        result.cull_mode = None;
        result
    }
}
