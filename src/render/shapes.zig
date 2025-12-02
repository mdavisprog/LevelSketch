const core = @import("core");
const render = @import("root.zig");
const std = @import("std");
const vertex_buffer = @import("vertex_buffer.zig");

const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;
const Vec3f = core.math.Vec3f;

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
            .setUVVec2(uvs[i]);
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

fn quarterArcPoints(
    center: Vec2f,
    radius: f32,
    angle: f32,
) [arc_vertex_count]Vec3f {
    var points = [_]Vec3f{.zero} ** arc_vertex_count;
    points[0] = center.xyz(0.0);

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

        points[point_index + 0] = p1.xyz(0.0);
        points[point_index + 1] = p2.xyz(0.0);

        point_index += 2;
    }

    return points;
}

pub fn quadPoints(rect: Rectf) [4]Vec3f {
    var points = [_]Vec3f{.zero} ** 4;
    points[0] = rect.min.xyz(0.0);
    points[1] = .init(rect.min.x, rect.max.y, 0.0);
    points[2] = rect.max.xyz(0.0);
    points[3] = .init(rect.max.x, rect.min.y, 0.0);
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
            points: []const Vec3f,
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
