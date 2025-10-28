const std = @import("std");
const zbgfx = @import("zbgfx");

const Self = @This();

x: f32 = 0.0,
y: f32 = 0.0,
z: f32 = 0.0,
u: f32 = 0.0,
v: f32 = 0.0,
abgr: u32 = 0xFFFFFFFF,

pub fn init(x: f32, y: f32, z: f32, u: f32, v: f32, abgr: u32) Self {
    return Self{
        .x = x,
        .y = y,
        .z = z,
        .u = u,
        .v = v,
        .abgr = abgr,
    };
}

/// Only a single instance of this is needed when creating the vertex buffer.
pub const Layout = struct {
    data: zbgfx.bgfx.VertexLayout,

    pub fn init() Layout {
        var data = std.mem.zeroes(zbgfx.bgfx.VertexLayout);
        data.begin(zbgfx.bgfx.RendererType.Noop)
            .add(.Position, 3, .Float, false, false)
            .add(.TexCoord0, 2, .Float, true, false)
            .add(.Color0, 4, .Uint8, true, true)
            .end();

        return Layout{
            .data = data,
        };
    }
};

test "ensure layout" {
    try std.testing.expectEqual(16, @sizeOf(Self));
}
