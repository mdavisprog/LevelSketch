const io = @import("io");
const stb = @import("stb");
const std = @import("std");
const Texture = @import("Texture.zig");
const zbgfx = @import("zbgfx");

const Self = @This();

pub const Error = error{
    InvalidHandle,
};

collection: std.ArrayList(Texture),
_allocator: std.mem.Allocator,
_allocations: std.ArrayList(Allocation),
_id: u32 = 1,
_mutex: std.Thread.Mutex = .{},

pub fn init(allocator: std.mem.Allocator) !Self {
    return Self{
        .collection = try std.ArrayList(Texture).initCapacity(allocator, 0),
        ._allocator = allocator,
        ._allocations = try std.ArrayList(Allocation).initCapacity(allocator, 0),
    };
}

pub fn deinit(self: *Self) void {
    for (self.collection.items) |texture| {
        if (texture.handle) |handle| {
            zbgfx.bgfx.destroyTexture(handle);
        }
    }
    self.collection.deinit(self._allocator);

    for (self._allocations.items) |alloc| {
        alloc.free(self._allocator);
    }
    self._allocations.deinit(self._allocator);
}

/// 'path' should be relative
pub fn load_image(self: *Self, allocator: std.mem.Allocator, path: []const u8) !Texture {
    const full_path = try io.exeRelativePath(allocator, &.{path});
    defer allocator.free(full_path);

    const contents = try io.getContents(allocator, full_path);
    defer allocator.free(contents);

    const data = try stb.image.load_from_memory(contents);

    return self.load(
        data.data,
        data.size(),
        data.width,
        data.height,
        Texture.Format.rgba8,
        .image,
    );
}

pub fn load_buffer(
    self: *Self,
    buffer: []const u8,
    width: u16,
    height: u16,
    format: Texture.Format,
) !Texture {
    return self.load(
        buffer.ptr,
        buffer.len,
        width,
        height,
        format,
        .buffer,
    );
}

pub fn load_static_buffer(
    self: *Self,
    buffer: []const u8,
    width: u16,
    height: u16,
    format: Texture.Format,
) !Texture {
    return self.load(
        buffer.ptr,
        buffer.len,
        width,
        height,
        format,
        .static_buffer,
    );
}

/// Should be called every frame to clean-up allocations that have been pushed to the GPU.
pub fn update(self: *Self) void {
    if (self._mutex.tryLock()) {
        defer self._mutex.unlock();

        for (self._allocations.items, 0..) |alloc, i| {
            if (alloc.state == .complete) {
                alloc.free(self._allocator);
                _ = self._allocations.swapRemove(i);
                break;
            }
        }
    }
}

fn load(
    self: *Self,
    buffer: [*]const u8,
    length: usize,
    width: u16,
    height: u16,
    format: Texture.Format,
    context: AllocationContext,
) !Texture {
    const id = self._id;
    var handler = try self._allocator.alloc(AllocationHandler, 1);
    handler[0].texture = id;
    handler[0].textures = self;

    self._id += 1;

    const mem = zbgfx.bgfx.makeRefRelease(
        buffer,
        @intCast(length),
        @ptrCast(@constCast(&onReleaseRef)),
        @ptrCast(handler.ptr),
    );

    const fmt: zbgfx.bgfx.TextureFormat = switch (format) {
        .grayscale, .rgba8 => .RGBA8,
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

    try self.addAllocation(id, buffer[0..length], context, handler);
    try self.collection.append(self._allocator, result);
    return result;
}

fn addAllocation(
    self: *Self,
    texture: Texture.Id,
    data: []const u8,
    context: AllocationContext,
    handler: []AllocationHandler,
) !void {
    self._mutex.lock();
    defer self._mutex.unlock();

    try self._allocations.append(self._allocator, .{
        .texture = texture,
        .data = data,
        .context = context,
        .handler = handler,
    });
}

/// May be called on the render thread.
fn finishAllocation(self: *Self, texture: Texture.Id) void {
    self._mutex.lock();
    defer self._mutex.unlock();

    for (self._allocations.items) |*alloc| {
        if (alloc.texture == texture) {
            alloc.*.state = .complete;
            break;
        }
    }
}

fn onReleaseRef(ptr: ?*anyopaque, context: ?*anyopaque) callconv(.c) void {
    _ = ptr;
    var handler: *AllocationHandler = @ptrCast(@alignCast(context.?));
    handler.finish();
}

const AllocationContext = enum {
    image,
    buffer,
    static_buffer,
};

/// A heap allocated struct to manage the allocation state of a texture. onReleasedRef may be
/// called from the render thread, so the updated state of the application needs to be
/// marshalled back to the main thread.
const AllocationHandler = struct {
    textures: *Self,
    texture: Texture.Id,

    fn finish(self: *AllocationHandler) void {
        self.textures.finishAllocation(self.texture);
    }
};

/// Stores a single texture allocation. The memory is held until the render thread has successfully
/// pushed the data to the GPU.
const Allocation = struct {
    pub const State = enum {
        loading,
        complete,
    };

    texture: Texture.Id,
    data: []const u8,
    context: AllocationContext,
    handler: []AllocationHandler,
    state: State = .loading,

    fn free(self: Allocation, allocator: std.mem.Allocator) void {
        switch (self.context) {
            .image => {
                stb.image.free_data(self.data);
            },
            .buffer => {
                allocator.free(self.data);
            },
            .static_buffer => {},
        }
        allocator.free(self.handler);
    }
};
