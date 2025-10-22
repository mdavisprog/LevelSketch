const std = @import("std");
const zbgfx = @import("zbgfx");

const Self = @This();

x: f32 = 0.0,
y: f32 = 0.0,
z: f32 = 0.0,
abgr: u32 = 0xFFFFFFFF,

pub fn init(x: f32, y: f32, z: f32, abgr: u32) Self {
    return Self{
        .x = x,
        .y = y,
        .z = z,
        .abgr = abgr,
    };
}

/// Only a single instance of this is needed when creating the vertex buffer.
pub const Layout = struct {
    data: zbgfx.bgfx.VertexLayout,

    pub fn init() Layout {
        var data = std.mem.zeroes(zbgfx.bgfx.VertexLayout);
        data.begin(zbgfx.bgfx.RendererType.Noop)
            .add(zbgfx.bgfx.Attrib.Position, 3, zbgfx.bgfx.AttribType.Float, false, false)
            .add(zbgfx.bgfx.Attrib.Color0, 4, zbgfx.bgfx.AttribType.Uint8, true, true)
            .end();

        return Layout{
            .data = data,
        };
    }
};

test "ensure layout" {
    try std.testing.expectEqual(16, @sizeOf(Self));
}
