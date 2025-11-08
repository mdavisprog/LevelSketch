const io = @import("io");
const MemFactory = @import("MemFactory.zig");
const stb = @import("stb");
const std = @import("std");
const Texture = @import("Texture.zig");
const zbgfx = @import("zbgfx");

const Self = @This();

pub const Error = error{
    InvalidHandle,
};

collection: std.ArrayList(Texture),
_id: u32 = 1,

pub fn init(allocator: std.mem.Allocator) !Self {
    return Self{
        .collection = try std.ArrayList(Texture).initCapacity(allocator, 0),
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    for (self.collection.items) |texture| {
        if (texture.handle) |handle| {
            zbgfx.bgfx.destroyTexture(handle);
        }
    }
    self.collection.deinit(allocator);
}

/// 'path' should be relative
pub fn load_image(self: *Self, mem_factory: *MemFactory, path: []const u8) !Texture {
    const allocator = mem_factory.allocator;

    const full_path = try io.exeRelativePath(allocator, &.{path});
    defer allocator.free(full_path);

    const contents = try io.getContents(allocator, full_path);
    defer allocator.free(contents);

    const data = try stb.image.load_from_memory(contents);

    return self.load(
        mem_factory,
        .stb_image,
        data.data,
        data.size(),
        data.width,
        data.height,
        Texture.Format.rgba8,
    );
}

pub fn load_buffer(
    self: *Self,
    mem_factory: *MemFactory,
    buffer: []const u8,
    width: u16,
    height: u16,
    format: Texture.Format,
) !Texture {
    return self.load(
        mem_factory,
        .buffer,
        buffer.ptr,
        buffer.len,
        width,
        height,
        format,
    );
}

pub fn load_static_buffer(
    self: *Self,
    mem_factory: *MemFactory,
    buffer: []const u8,
    width: u16,
    height: u16,
    format: Texture.Format,
) !Texture {
    return self.load(
        mem_factory,
        .static_buffer,
        buffer.ptr,
        buffer.len,
        width,
        height,
        format,
    );
}

fn load(
    self: *Self,
    mem_factory: *MemFactory,
    context: MemFactory.Context,
    buffer: [*]const u8,
    length: usize,
    width: u16,
    height: u16,
    format: Texture.Format,
) !Texture {
    const id = self._id;
    self._id += 1;

    const slice = buffer[0..length];
    const mem = try mem_factory.create(slice, context);

    const fmt: zbgfx.bgfx.TextureFormat = switch (format) {
        .grayscale => .R8,
        .rgba8 => .RGBA8,
    };

    const handle = zbgfx.bgfx.createTexture2D(
        width,
        height,
        false,
        1,
        fmt,
        zbgfx.bgfx.TextureFlags_None,
        mem,
    );

    const result = Texture{
        .id = id,
        .handle = handle,
        .width = width,
        .height = height,
        .format = format,
    };

    try self.collection.append(mem_factory.allocator, result);
    return result;
}
