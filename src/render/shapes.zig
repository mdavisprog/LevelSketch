const core = @import("core");
const render = @import("root.zig");
const std = @import("std");

const Rectf = core.math.Rectf;

const VertexBuffer16 = render.VertexBuffer16;

pub fn quad(allocator: std.mem.Allocator, rect: Rectf, color: u32) !VertexBuffer16 {
    var result: VertexBuffer16 = try .init(allocator, 4, 6);

    _ = result.vertices.items[0].setPositionVec2(rect.min);
    _ = result.vertices.items[1].setPositionVec2(.init(rect.min.x, rect.max.y));
    _ = result.vertices.items[2].setPositionVec2(rect.max);
    _ = result.vertices.items[3].setPositionVec2(.init(rect.max.x, rect.min.y));

    _ = result.vertices.items[0].setUV(0.0, 0.0);
    _ = result.vertices.items[1].setUV(0.0, 1.0);
    _ = result.vertices.items[2].setUV(1.0, 1.0);
    _ = result.vertices.items[3].setUV(1.0, 0.0);

    _ = result.vertices.items[0].setColor(color);
    _ = result.vertices.items[1].setColor(color);
    _ = result.vertices.items[2].setColor(color);
    _ = result.vertices.items[3].setColor(color);

    result.indices.items[0] = 0;
    result.indices.items[1] = 1;
    result.indices.items[2] = 2;
    result.indices.items[3] = 0;
    result.indices.items[4] = 2;
    result.indices.items[5] = 3;

    return result;
}
