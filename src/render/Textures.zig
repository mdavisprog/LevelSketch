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
default: Texture = .{},
_id: Texture.Id = 1,
_uploads: std.ArrayList(*Upload),

pub fn init(factory: *MemFactory) !Self {
    var result = Self{
        .collection = try std.ArrayList(Texture).initCapacity(factory.allocator, 0),
        ._uploads = try std.ArrayList(*Upload).initCapacity(factory.allocator, 0),
    };

    result.default = try result.loadStaticBuffer(
        factory,
        &.{ 255, 255, 255, 255 },
        1,
        1,
        .rgba8,
    );

    return result;
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    for (self.collection.items) |texture| {
        if (texture.handle) |handle| {
            zbgfx.bgfx.destroyTexture(handle);
        }
    }
    self.collection.deinit(allocator);

    for (self._uploads.items) |item| {
        allocator.destroy(item);
    }
    self._uploads.deinit(allocator);
}

/// 'path' should be relative
pub fn loadImage(self: *Self, mem_factory: *MemFactory, path: []const u8) !Texture {
    const allocator = mem_factory.allocator;

    const full_path = try io.exeRelativePath(allocator, &.{path});
    defer allocator.free(full_path);

    return self.loadImageAbsolute(mem_factory, full_path);
}

pub fn loadImageAbsolute(self: *Self, mem_factory: *MemFactory, path: []const u8) !Texture {
    const allocator = mem_factory.allocator;

    const contents = try io.getContents(allocator, path);
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

pub fn loadBuffer(
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

pub fn loadStaticBuffer(
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
    buffer_type: BufferType,
    buffer: [*]const u8,
    length: usize,
    width: u16,
    height: u16,
    format: Texture.Format,
) !Texture {
    const id = self._id;
    self._id += 1;

    const slice = buffer[0..length];

    const mem = blk: {
        switch (buffer_type) {
            .stb_image, .buffer => {
                const upload = try mem_factory.allocator.create(Upload);
                upload.*.id = id;
                upload.*.textures = self;
                upload.*.buffer_type = buffer_type;

                try self._uploads.append(mem_factory.allocator, upload);
                break :blk try mem_factory.create(
                    slice,
                    onUploadedCallback,
                    @ptrCast(upload),
                );
            },
            .static_buffer => {
                break :blk try mem_factory.create(slice, null, null);
            },
        }
    };

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
        mem.ptr,
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

/// Called on the main thread. No need to worry about a mutex.
fn onUploaded(self: *Self, allocator: std.mem.Allocator, id: Texture.Id) void {
    for (self._uploads.items, 0..) |upload, i| {
        if (upload.id == id) {
            const removed = self._uploads.swapRemove(i);
            allocator.destroy(removed);
            break;
        }
    }
}

fn onUploadedCallback(result: MemFactory.OnUploadedResult) void {
    const upload: *Upload = result.userDataAs(Upload);

    switch (upload.buffer_type) {
        .stb_image => {
            stb.image.free_data(result.data);
        },
        .buffer => {
            result.allocator.free(result.data);
        },
        .static_buffer => {},
    }

    upload.textures.onUploaded(result.allocator, upload.id);
}

const BufferType = enum {
    stb_image,
    buffer,
    static_buffer,
};

const Upload = struct {
    textures: *Self,
    id: Texture.Id,
    buffer_type: BufferType,
};
