const core = @import("core");
const render = @import("root.zig");
const std = @import("std");
const vertex_buffer = @import("vertex_buffer.zig");

const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;
const Vec = core.math.Vec;

const Mesh = render.Mesh;
const Renderer = render.Renderer;
const Vertex = render.Vertex;
const VertexBuffer = vertex_buffer.VertexBuffer;

pub const arc_segments: usize = 16;
pub const arc_vertex_count: usize = 1 + arc_segments * 2;

pub fn quad(
    comptime IndexType: type,
    allocator: std.mem.Allocator,
    rect: Rectf,
    color: u32,
) !VertexBuffer(IndexType) {
    var builder: Builder(IndexType) = try .init(allocator);

    const points = quadPoints(rect);
    const indices = quadIndices(IndexType);
    const uvs = [_]Vec2f{ .zero, .init(0.0, 1.0), .splat(1.0), .init(1.0, 0.0) };

    var vertices = try builder.addPoints(&points, &indices);
    for (0..uvs.len) |i| {
        _ = vertices[i]
            .setColor(color)
            .setUVVec2(uvs[i].toVec());
    }

    return builder.buffer;
}

pub fn quadRounded(
    comptime IndexType: type,
    allocator: std.mem.Allocator,
    rect: Rectf,
    color: u32,
    corners: [4]f32,
) !VertexBuffer(IndexType) {
    var builder: Builder(IndexType) = try .init(allocator);

    const tl_radius = corners[0];
    const tr_radius = corners[1];
    const bl_radius = corners[2];
    const br_radius = corners[3];

    const tl: Vec2f = rect.min;
    const tr: Vec2f = .init(rect.max.x, rect.min.y);
    const bl: Vec2f = .init(rect.min.x, rect.max.y);
    const br: Vec2f = rect.max;

    const quad_indices = quadIndices(IndexType);

    // Middle rect
    const tl_middle = tl.add(.init(tl_radius, 0.0));
    const br_middle = br.sub(.init(br_radius, 0.0));
    _ = try builder.addPoints(
        &quadPoints(.{ .min = tl_middle, .max = br_middle }),
        &quad_indices,
    );

    // Left rect
    const tl_left = tl.add(.init(0.0, tl_radius));
    const br_left = bl.add(.init(bl_radius, -bl_radius));
    _ = try builder.addPoints(
        &quadPoints(.{ .min = tl_left, .max = br_left }),
        &quad_indices,
    );

    // Right rect
    const tl_right = tr.add(.init(-tr_radius, tr_radius));
    const br_right = br.add(.init(0.0, -br_radius));
    _ = try builder.addPoints(
        &quadPoints(.{ .min = tl_right, .max = br_right }),
        &quad_indices,
    );

    const quarter_arc_indices = quarterArcIndices(IndexType, arc_segments);
    // Top-left corner
    _ = try builder.addPoints(
        &quarterArcPoints(tl.add(.splat(tl_radius)), tl_radius, 180.0),
        &quarter_arc_indices,
    );

    // Top-right corner
    _ = try builder.addPoints(
        &quarterArcPoints(tr.add(.init(-tr_radius, tr_radius)), tr_radius, 270.0),
        &quarter_arc_indices,
    );

    // Bottom-left corner
    _ = try builder.addPoints(
        &quarterArcPoints(bl.add(.init(bl_radius, -bl_radius)), bl_radius, 90.0),
        &quarter_arc_indices,
    );

    // Bottom-right corner
    _ = try builder.addPoints(
        &quarterArcPoints(br.add(.splat(-br_radius)), br_radius, 0.0),
        &quarter_arc_indices,
    );

    fillColor(builder.buffer.vertices.items[0..], color);
    // TODO: Update UVs

    return builder.buffer;
}

pub fn cube(
    comptime IndexType: type,
    renderer: *Renderer,
    half_size: Vec,
    color: u32,
) !Mesh {
    const buffer = try cubeBuffer(IndexType, renderer.allocator, half_size, color);
    return try Mesh.initWithBuffer(renderer, buffer);
}

pub fn cubeBuffer(
    comptime IndexType: type,
    allocator: std.mem.Allocator,
    half_size: Vec,
    color: u32,
) !VertexBuffer(IndexType) {
    const min = half_size.mul(-1.0);
    const max = half_size;

    // TODO: Add Normals. Will need 24 verts for this.
    const vertices = [_]Vertex{
        // Front face
        .init(.init3(min.x(), max.y(), min.z()), .init3(0.0, 0.0, -1.0), .init2(0.0, 0.0), color),
        .init(.init3(min.x(), min.y(), min.z()), .init3(0.0, 0.0, -1.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), min.y(), min.z()), .init3(0.0, 0.0, -1.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), max.y(), min.z()), .init3(0.0, 0.0, -1.0), .init2(0.0, 0.0), color),

        // Back face
        .init(.init3(max.x(), max.y(), max.z()), .init3(0.0, 0.0, 1.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), min.y(), max.z()), .init3(0.0, 0.0, 1.0), .init2(0.0, 0.0), color),
        .init(.init3(min.x(), min.y(), max.z()), .init3(0.0, 0.0, 1.0), .init2(0.0, 0.0), color),
        .init(.init3(min.x(), max.y(), max.z()), .init3(0.0, 0.0, 1.0), .init2(0.0, 0.0), color),

        // Right face
        .init(.init3(max.x(), max.y(), min.z()), .init3(1.0, 0.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), min.y(), min.z()), .init3(1.0, 0.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), min.y(), max.z()), .init3(1.0, 0.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), max.y(), max.z()), .init3(1.0, 0.0, 0.0), .init2(0.0, 0.0), color),

        // Left face
        .init(.init3(min.x(), max.y(), max.z()), .init3(-1.0, 0.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(min.x(), min.y(), max.z()), .init3(-1.0, 0.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(min.x(), min.y(), min.z()), .init3(-1.0, 0.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(min.x(), max.y(), min.z()), .init3(-1.0, 0.0, 0.0), .init2(0.0, 0.0), color),

        // Top face
        .init(.init3(min.x(), max.y(), max.z()), .init3(0.0, 1.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(min.x(), max.y(), min.z()), .init3(0.0, 1.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), max.y(), min.z()), .init3(0.0, 1.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), max.y(), max.z()), .init3(0.0, 1.0, 0.0), .init2(0.0, 0.0), color),

        // Bottom face
        .init(.init3(min.x(), min.y(), min.z()), .init3(0.0, -1.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(min.x(), min.y(), max.z()), .init3(0.0, -1.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), min.y(), max.z()), .init3(0.0, -1.0, 0.0), .init2(0.0, 0.0), color),
        .init(.init3(max.x(), min.y(), min.z()), .init3(0.0, -1.0, 0.0), .init2(0.0, 0.0), color),
    };
    // 6 points per face, 6 faces
    const indices = [36]IndexType{
        0,  1,  2,  0,  2,  3,
        4,  5,  6,  4,  6,  7,
        8,  9,  10, 8,  10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23,
    };

    const buffer: VertexBuffer(IndexType) = try .init(allocator, vertices.len, indices.len);
    @memcpy(buffer.vertices.items, &vertices);
    @memcpy(buffer.indices.items, &indices);

    return buffer;
}

fn quarterArcPoints(
    center: Vec2f,
    radius: f32,
    angle: f32,
) [arc_vertex_count]Vec {
    var points = [_]Vec{.zero} ** arc_vertex_count;
    points[0] = center.toVec();

    const segments: f32 = @floatFromInt(arc_segments);
    const step: f32 = std.math.degreesToRadians(90.0 / segments);
    const angle_rad: f32 = std.math.degreesToRadians(angle);

    // Offset 1 from the center.
    var point_index: usize = 1;
    var i: usize = 0;
    while (i < arc_segments) {
        defer i += 1;

        const angle_1: f32 = @as(f32, @floatFromInt(i)) * step + angle_rad;
        const angle_2: f32 = @as(f32, @floatFromInt(i + 1)) * step + angle_rad;

        const p1 = center.add(.init(
            std.math.cos(angle_1) * radius,
            std.math.sin(angle_1) * radius,
        ));
        const p2 = center.add(.init(
            std.math.cos(angle_2) * radius,
            std.math.sin(angle_2) * radius,
        ));

        points[point_index + 0] = p1.toVec();
        points[point_index + 1] = p2.toVec();

        point_index += 2;
    }

    return points;
}

pub fn quadPoints(rect: Rectf) [4]Vec {
    var points = [_]Vec{.zero} ** 4;
    points[0] = rect.min.toVec();
    points[1] = .init2(rect.min.x, rect.max.y);
    points[2] = rect.max.toVec();
    points[3] = .init2(rect.max.x, rect.min.y);
    return points;
}

pub fn quadIndices(comptime IndexType: type) [6]IndexType {
    return [6]IndexType{ 0, 1, 2, 0, 2, 3 };
}

fn quarterArcIndices(comptime IndexType: type, comptime segments: usize) [segments * 3]IndexType {
    const count = segments * 3;
    var indices = [_]IndexType{0} ** count;
    var vertex_index: IndexType = 1;
    var index: usize = 0;

    for (0..segments) |_| {
        indices[index + 0] = 0;
        indices[index + 1] = vertex_index + 0;
        indices[index + 2] = vertex_index + 1;

        vertex_index += 2;
        index += 3;
    }

    return indices;
}

fn verifyIndexType(comptime IndexType: type) void {
    if (IndexType != u16 and IndexType != u32) {
        @compileError(std.fmt.comptimePrint("IndexType is not a u16 or u32. Given type is {s}.", .{@typeName(IndexType)}));
    }
}

fn Builder(comptime IndexType: type) type {
    verifyIndexType(IndexType);

    return struct {
        const Self = @This();

        allocator: std.mem.Allocator,
        buffer: VertexBuffer(IndexType),

        fn init(allocator: std.mem.Allocator) !Self {
            return .{
                .allocator = allocator,
                .buffer = try .init(allocator, 0, 0),
            };
        }

        fn addPoints(
            self: *Self,
            points: []const Vec,
            indices: []const IndexType,
        ) ![]Vertex {
            const base_v_index = self.buffer.vertices.items.len;
            const base_i_index = self.buffer.indices.items.len;

            try self.buffer.vertices.appendNTimes(self.allocator, .{}, points.len);
            try self.buffer.indices.appendNTimes(self.allocator, 0, indices.len);

            for (0..points.len) |i| {
                const point = points[i];
                _ = self.buffer.vertices.items[base_v_index + i].setPositionVec3(point);
            }

            const offset: IndexType = @intCast(base_v_index);
            for (0..indices.len) |i| {
                const index = indices[i];
                self.buffer.indices.items[base_i_index + i] = offset + index;
            }

            return self.buffer.vertices.items[base_v_index..];
        }
    };
}

fn fillColor(vertices: []Vertex, color: u32) void {
    for (vertices) |*vertex| {
        _ = vertex.setColor(color);
    }
}
