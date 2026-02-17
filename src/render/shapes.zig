const core = @import("core");
const render = @import("root.zig");
const std = @import("std");
const vertex_buffer = @import("vertex_buffer.zig");

const Meshes = render.Meshes;
const Rectf = core.math.Rectf;
const Renderer = render.Renderer;
const UIVertex = render.UIVertex;
const Vec = core.math.Vec;
const Vec2f = core.math.Vec2f;
const Vertex = render.Vertex;
const VertexBuffer = vertex_buffer.VertexBuffer;

pub const arc_segments: usize = 16;
pub const arc_vertex_count: usize = 1 + arc_segments * 2;

pub fn quad(
    comptime IndexType: type,
    allocator: std.mem.Allocator,
    rect: Rectf,
    color: u32,
) !VertexBuffer(UIVertex, IndexType) {
    var builder: Builder(UIVertex, IndexType) = try .init(allocator);

    const points = quadPoints(rect);
    const indices = quadIndices(IndexType);
    const uvs = [_]Vec2f{ .zero, .init(0.0, 1.0), .splat(1.0), .init(1.0, 0.0) };

    var vertices = try builder.addPoints(&points, &indices);
    for (0..uvs.len) |i| {
        vertices[i].abgr = color;
        vertices[i].setUV(uvs[i]);
    }

    return builder.buffer;
}

pub fn quadRounded(
    comptime IndexType: type,
    allocator: std.mem.Allocator,
    rect: Rectf,
    color: u32,
    corners: [4]f32,
) !VertexBuffer(UIVertex, IndexType) {
    var builder: Builder(UIVertex, IndexType) = try .init(allocator);

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
) !Meshes.Mesh.Handle {
    var buffer = try cubeBuffer(IndexType, renderer.allocator, half_size, color);
    errdefer buffer.deinit(renderer.allocator);

    return renderer.loadMeshFromBuffer(buffer);
}

pub fn cubeBuffer(
    comptime IndexType: type,
    allocator: std.mem.Allocator,
    half_size: Vec,
    color: u32,
) !VertexBuffer(Vertex, IndexType) {
    const min = half_size.mul(-1.0);
    const max = half_size;

    // TODO: Add UVs.
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

    const buffer: VertexBuffer(Vertex, IndexType) = try .init(allocator, vertices.len, indices.len);
    @memcpy(buffer.vertices.items, &vertices);
    @memcpy(buffer.indices.items, &indices);

    return buffer;
}

pub const Cylinder = struct {
    radius: f32 = 0.5,
    half_height: f32 = 0.5,
    resolution: u32 = 32,
    segments: u32 = 1,
};

pub fn cylinder(
    comptime IndexType: type,
    renderer: *Renderer,
    settings: Cylinder,
) !VertexBuffer(Vertex, IndexType) {
    const allocator = renderer.allocator;

    std.debug.assert(settings.resolution > 2);
    std.debug.assert(settings.segments > 0);

    const num_rings = settings.segments + 1;
    const step_theta: f32 = std.math.tau / @as(f32, @floatFromInt(settings.resolution));
    const step_y = 2.0 * settings.half_height / @as(f32, @floatFromInt(settings.segments));

    var buffer: VertexBuffer(Vertex, IndexType) = try .init(allocator, 0, 0);
    errdefer buffer.deinit(allocator);

    // rings

    for (0..num_rings) |ring| {
        const y = -settings.half_height + @as(f32, @floatFromInt(ring)) * step_y;

        var segment: u32 = 0;
        while (segment <= settings.resolution) {
            defer segment += 1;

            const fsegment: f32 = @floatFromInt(segment);
            const theta = fsegment * step_theta;
            const sin, const cos = core.math.sinCos(theta);

            const vertex: Vertex = .init(
                .init3(settings.radius * cos, y, settings.radius * sin),
                .init3(cos, 0.0, sin),
                .init2(
                    fsegment / @as(f32, @floatFromInt(settings.resolution)),
                    @as(f32, @floatFromInt(ring)) / fsegment,
                ),
                0xFFFFFFFF,
            );
            try buffer.vertices.append(allocator, vertex);
        }
    }

    // barrel skin

    for (0..settings.segments) |i| {
        const _i: u32 = @intCast(i);
        const ring = _i * (settings.resolution + 1);
        const next_ring = (_i + 1) * (settings.resolution + 1);

        for (0..settings.resolution) |j| {
            const _j: u32 = @intCast(j);
            try buffer.indices.appendSlice(allocator, &.{
                @intCast(ring + _j),
                @intCast(ring + _j + 1),
                @intCast(next_ring + _j),
                @intCast(next_ring + _j),
                @intCast(ring + _j + 1),
                @intCast(next_ring + _j + 1),
            });
        }
    }

    // caps

    try cylinderBuildCap(IndexType, allocator, settings, &buffer, true, step_theta);
    try cylinderBuildCap(IndexType, allocator, settings, &buffer, false, step_theta);

    return buffer;
}

pub fn cylinderMesh(
    comptime IndexType: type,
    renderer: *Renderer,
    settings: Cylinder,
) !Meshes.Mesh.Handle {
    const buffer = try cylinder(IndexType, renderer, settings);
    return try renderer.loadMeshFromBuffer(buffer);
}

fn cylinderBuildCap(
    comptime IndexType: type,
    allocator: std.mem.Allocator,
    settings: Cylinder,
    buffer: *VertexBuffer(Vertex, IndexType),
    top: bool,
    step_theta: f32,
) !void {
    const offset: u32 = @intCast(buffer.vertices.items.len);
    const y, const normal_y: f32, const winding: struct { u32, u32 } = if (top)
        .{ settings.half_height, 1.0, .{ @as(u32, 1), 0 } }
    else
        .{ -settings.half_height, -1.0, .{ 0, 1 } };

    for (0..settings.resolution) |i| {
        const theta = @as(f32, @floatFromInt(i)) * step_theta;
        const sin, const cos = core.math.sinCos(theta);

        const vertex: Vertex = .init(
            .init3(cos * settings.radius, y, sin * settings.radius),
            .init3(0.0, normal_y, 0.0),
            .init2(0.5 * (cos + 1.0), 1.0 - 0.5 * (sin + 1.0)),
            0xFFFFFFFF,
        );
        try buffer.vertices.append(allocator, vertex);
    }

    for (1..(settings.resolution - 1)) |i| {
        const _i: u32 = @intCast(i);
        try buffer.indices.appendSlice(allocator, &.{
            @intCast(offset),
            @intCast(offset + _i + winding.@"1"),
            @intCast(offset + _i + winding.@"0"),
        });
    }
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

fn Builder(comptime VertexType: type, comptime IndexType: type) type {
    return struct {
        const Self = @This();

        allocator: std.mem.Allocator,
        buffer: VertexBuffer(VertexType, IndexType),

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
        ) ![]VertexType {
            const base_v_index = self.buffer.vertices.items.len;
            const base_i_index = self.buffer.indices.items.len;

            try self.buffer.vertices.appendNTimes(self.allocator, .{}, points.len);
            try self.buffer.indices.appendNTimes(self.allocator, 0, indices.len);

            for (0..points.len) |i| {
                const point = points[i];

                if (VertexType == Vertex) {
                    _ = self.buffer.vertices.items[base_v_index + i].setPositionVec3(point);
                } else {
                    self.buffer.vertices.items[base_v_index + i].setPosition(point);
                }
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

fn fillColor(vertices: anytype, color: u32) void {
    const Type = @TypeOf(vertices);

    if (Type == []Vertex) {
        for (vertices) |*vertex| {
            _ = vertex.setColor(color);
        }
    } else if (Type == []UIVertex) {
        for (vertices) |*vertex| {
            vertex.abgr = color;
        }
    } else {
        @compileError(std.fmt.comptimePrint("Unsupported type {s}.", .{@typeName(Type)}));
    }
}
