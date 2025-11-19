const core = @import("core");
const render = @import("root.zig");
const std = @import("std");

const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;

const Vertex = render.Vertex;
const VertexBuffer16 = render.VertexBuffer16;
const VertexBufferBuilder16 = render.VertexBufferBuilder16;

pub fn quad(allocator: std.mem.Allocator, rect: Rectf, color: u32) !VertexBuffer16 {
    var result: VertexBufferBuilder16 = try .init(allocator);
    try result.addQuad(rect, color);
    return result.buffer;
}

pub fn quadRounded(
    allocator: std.mem.Allocator,
    rect: Rectf,
    color: u32,
    corners: [4]f32,
) !VertexBuffer16 {
    var result: VertexBufferBuilder16 = try .init(allocator);

    const tl_radius = corners[0];
    const tr_radius = corners[1];
    const bl_radius = corners[2];
    const br_radius = corners[3];

    const tl: Vec2f = rect.min;
    const tr: Vec2f = .init(rect.max.x, rect.min.y);
    const bl: Vec2f = .init(rect.min.x, rect.max.y);
    const br: Vec2f = rect.max;

    // Middle rect
    const tl_middle = tl.add(.init(tl_radius, 0.0));
    const br_middle = br.sub(.init(br_radius, 0.0));
    try result.addQuad(.{ .min = tl_middle, .max = br_middle }, color);

    // Left rect
    const tl_left = tl.add(.init(0.0, tl_radius));
    const br_left = bl.add(.init(bl_radius, -bl_radius));
    try result.addQuad(.{ .min = tl_left, .max = br_left }, color);

    // Right rect
    const tl_right = tr.add(.init(-tr_radius, tr_radius));
    const br_right = br.add(.init(0.0, -br_radius));
    try result.addQuad(.{ .min = tl_right, .max = br_right }, color);

    // Top-left corner
    try result.addQuarterArc(tl.add(.splat(tl_radius)), tl_radius, 180.0, color);

    // Top-right corner
    try result.addQuarterArc(tr.add(.init(-tr_radius, tr_radius)), tr_radius, 270.0, color);

    // Bottom-left corner
    try result.addQuarterArc(bl.add(.init(bl_radius, -bl_radius)), bl_radius, 90.0, color);

    // Bottom-right corner
    try result.addQuarterArc(br.add(.splat(-br_radius)), br_radius, 0.0, color);

    return result.buffer;
}
