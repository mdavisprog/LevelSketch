use bevy::prelude::*;
use bevy::render::view::RenderLayers;
use crate::tools::constants;
use super::*;

#[derive(Component)]
#[require(Transform, Visibility)]
pub struct Translation;

impl Translation {
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

        let entity = commands.spawn((
            Self,
        ))
        .with_children(|parent| {
            // X-Axis
            parent.spawn((
                layer.clone(),
                Mesh3d(cylinder_mesh.clone()),
                MeshMaterial3d(materials.add(Self::standard_material(constants::X_AXIS_COLOR))),
                axis::Axis {
                    color: constants::X_AXIS_COLOR,
                    direction: axis::Direction::X,
                },
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
                axis::Axis {
                    color: constants::Y_AXIS_COLOR,
                    direction: axis::Direction::Y,
                },
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
                axis::Axis {
                    color: constants::Z_AXIS_COLOR,
                    direction: axis::Direction::Z,
                },
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
                MeshMaterial3d(materials.add(Self::standard_material_no_cull(constants::Y_AXIS_COLOR))),
                axis::Axis {
                    color: constants::Y_AXIS_COLOR,
                    direction: axis::Direction::XY,
                },
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
                MeshMaterial3d(materials.add(Self::standard_material_no_cull(constants::X_AXIS_COLOR))),
                axis::Axis {
                    color: constants::X_AXIS_COLOR,
                    direction: axis::Direction::XZ,
                },
                Transform::from_xyz(plane_offset, cylinder_base * -0.5, plane_offset),
                RayCastBackfaces,
            ));

            // YZ plane
            parent.spawn((
                layer.clone(),
                Mesh3d(plane_mesh.clone()),
                MeshMaterial3d(materials.add(Self::standard_material_no_cull(constants::Z_AXIS_COLOR))),
                axis::Axis {
                    color: constants::Z_AXIS_COLOR,
                    direction: axis::Direction::YZ,
                },
                Transform::from_xyz(
                    cylinder_base * -0.5, plane_offset, plane_offset)
                    .with_rotation(Quat::from_rotation_z(-90.0_f32.to_radians())
                ),
                RayCastBackfaces,
            ));
        }).id();

        entity
    }

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
