const core = @import("core");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Vec2f = core.math.Vec2f;
const Vec3f = core.math.Vec3f;

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

pub fn setPosition(self: *Self, x: f32, y: f32, z: f32) *Self {
    self.*.x = x;
    self.*.y = y;
    self.*.z = z;
    return self;
}

pub fn setPositionVec3(self: *Self, position: Vec3f) *Self {
    self.*.x = position.x;
    self.*.y = position.y;
    self.*.z = position.z;
    return self;
}

pub fn setPositionVec2(self: *Self, position: Vec2f) *Self {
    self.*.x = position.x;
    self.*.y = position.y;
    self.*.z = 0.0;
    return self;
}

pub fn setUV(self: *Self, u: f32, v: f32) *Self {
    self.*.u = u;
    self.*.v = v;
    return self;
}

pub fn setUVVec2(self: *Self, uv: Vec2f) *Self {
    self.*.u = uv.x;
    self.*.v = uv.y;
    return self;
}

pub fn setColor(self: *Self, abgr: u32) *Self {
    self.abgr = abgr;
    return self;
}

pub fn setColor4b(self: *Self, r: u8, g: u8, b: u8, a: u8) *Self {
    const _r: u32 = @intCast(r);
    const _g: u32 = @intCast(g);
    const _b: u32 = @intCast(b);
    const _a: u32 = @intCast(a);
    self.abgr = _r | std.math.shl(u32, _g, 8) | std.math.shl(u32, _b, 16) | std.math.shl(u32, _a, 24);
    return self;
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
    // TODO: Try to get this to work. Currently running into a linker error when trying to
    // reference 'zbgfx' properties.
    //const layout: Self.Layout = .init();
    //try std.testing.expectEqual(layout.data.stride, @sizeOf(Self));
    try std.testing.expectEqual(24, @sizeOf(Self));
}
