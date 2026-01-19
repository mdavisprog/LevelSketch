const render = @import("root.zig");
const zbgfx = @import("zbgfx");

const Self = @This();

pub const Handle = render.Handle(Self);

pub const Error = error{
    InvalidHandle,
};

pub const Format = enum {
    grayscale,
    rgba8,
};

handle: Handle = .invalid,
bgfx_handle: ?zbgfx.bgfx.TextureHandle = null,
width: u16 = 0,
height: u16 = 0,
format: Format = .rgba8,
flags: u32 = zbgfx.bgfx.TextureFlags_None,

pub fn deinit(self: *Self) void {
    if (self.bgfx_handle) |handle| {
        zbgfx.bgfx.destroyTexture(handle);
    }
}

pub fn bind(self: Self, sampler: zbgfx.bgfx.UniformHandle, flags: u32) !void {
    const handle = self.bgfx_handle orelse return Error.InvalidHandle;
    zbgfx.bgfx.setTexture(
        0,
        sampler,
        handle,
        flags,
    );
}
