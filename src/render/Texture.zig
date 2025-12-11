const zbgfx = @import("zbgfx");

const Self = @This();

pub const Error = error{
    InvalidHandle,
};

pub const Format = enum {
    grayscale,
    rgba8,
};

pub const Id = u32;
pub const invalid_id: u32 = 0;

id: Id = invalid_id,
handle: ?zbgfx.bgfx.TextureHandle = null,
width: u16 = 0,
height: u16 = 0,
format: Format = .rgba8,
flags: u32 = zbgfx.bgfx.TextureFlags_None,

pub fn bind(self: Self, sampler: zbgfx.bgfx.UniformHandle, flags: u32) !void {
    const handle = self.handle orelse return Error.InvalidHandle;
    zbgfx.bgfx.setTexture(
        0,
        sampler,
        handle,
        flags,
    );
}
