use bevy::asset::RenderAssetUsages;
use bevy::math::FloatPow;
use bevy::prelude::*;
use bevy::render::{
    mesh::Indices,
    render_resource::PrimitiveTopology,
};
use core::f32::consts::*;

#[derive(Clone, Copy, PartialEq)]
pub struct AnnulusSector {
    pub inner_circle: Circle,
    pub outer_circle: Circle,
    pub half_angle: f32,
}

impl Primitive2d for AnnulusSector {}

impl Default for AnnulusSector {
    fn default() -> Self {
        Self {
            inner_circle: Circle::new(0.5),
            outer_circle: Circle::new(1.0),
            half_angle: 1.0,
        }
    }
}

impl Measured2d for AnnulusSector {
    fn area(&self) -> f32 {
        PI * (self.outer_circle.radius.squared() - self.inner_circle.radius.squared())
    }

    fn perimeter(&self) -> f32 {
        2.0 * PI * (self.outer_circle.radius + self.inner_circle.radius)
    }
}

impl AnnulusSector {
    pub const fn new(inner_radius: f32, outer_radius: f32, half_angle: f32) -> Self {
        Self {
            inner_circle: Circle::new(inner_radius),
            outer_circle: Circle::new(outer_radius),
            half_angle,
        }
    }
}

pub struct AnnulusSectorMeshBuilder {
    pub annulus_sector: AnnulusSector,
    pub resolution: u32,
}

impl Default for AnnulusSectorMeshBuilder {
    fn default() -> Self {
        Self {
            annulus_sector: AnnulusSector::default(),
            resolution: 32,
        }
    }
}

impl MeshBuilder for AnnulusSectorMeshBuilder {
    fn build(&self) -> Mesh {
        let inner_radius = self.annulus_sector.inner_circle.radius;
        let outer_radius = self.annulus_sector.outer_circle.radius;

        let num_vertices = (self.resolution as usize + 1) * 2;
        let mut indices = Vec::with_capacity(self.resolution as usize * 6);
        let mut positions = Vec::with_capacity(num_vertices);
        let mut uvs = Vec::with_capacity(num_vertices);
        let normals = vec![[0.0, 0.0, 1.0]; num_vertices];

        // We have one more set of vertices than might be naïvely expected;
        // the vertices at `start_angle` are duplicated for the purposes of UV
        // mapping. Here, each iteration places a pair of vertices at a fixed
        // angle from the center of the annulus.
        let start_angle = FRAC_PI_2 - self.annulus_sector.half_angle;
        let end_angle = FRAC_PI_2 + self.annulus_sector.half_angle;
        let last_i = (self.resolution - 1) as f32;
        for i in 0..=self.resolution {
            let theta = f32::lerp(start_angle, end_angle, i as f32 / last_i);
            let (sin, cos) = ops::sin_cos(theta);
            let inner_pos = [cos * inner_radius, sin * inner_radius, 0.];
            let outer_pos = [cos * outer_radius, sin * outer_radius, 0.];
            positions.push(inner_pos);
            positions.push(outer_pos);

            // The first UV direction is radial and the second is angular;
            // i.e., a single UV rectangle is stretched around the annulus, with
            // its top and bottom meeting as the circle closes. Lines of constant
            // U map to circles, and lines of constant V map to radial line segments.
            let inner_uv = [0., i as f32 / self.resolution as f32];
            let outer_uv = [1., i as f32 / self.resolution as f32];
            uvs.push(inner_uv);
            uvs.push(outer_uv);
        }

        // Adjacent pairs of vertices form two triangles with each other; here,
        // we are just making sure that they both have the right orientation,
        // which is the CCW order of
        // `inner_vertex` -> `outer_vertex` -> `next_outer` -> `next_inner`
        for i in 0..self.resolution {
            let inner_vertex = 2 * i;
            let outer_vertex = 2 * i + 1;
            let next_inner = inner_vertex + 2;
            let next_outer = outer_vertex + 2;
            indices.extend_from_slice(&[inner_vertex, outer_vertex, next_outer]);
            indices.extend_from_slice(&[next_outer, next_inner, inner_vertex]);
        }

        Mesh::new(
            PrimitiveTopology::TriangleList,
            RenderAssetUsages::default(),
        )
        .with_inserted_attribute(Mesh::ATTRIBUTE_POSITION, positions)
        .with_inserted_attribute(Mesh::ATTRIBUTE_NORMAL, normals)
        .with_inserted_attribute(Mesh::ATTRIBUTE_UV_0, uvs)
        .with_inserted_indices(Indices::U32(indices))
    }
}

impl Meshable for AnnulusSector {
    type Output = AnnulusSectorMeshBuilder;

    fn mesh(&self) -> Self::Output {
        Self::Output {
            annulus_sector: *self,
            ..Default::default()
        }
    }
}

impl From<AnnulusSector> for Mesh {
    fn from(annulus_sector: AnnulusSector) -> Self {
        annulus_sector.mesh().build()
    }
}
