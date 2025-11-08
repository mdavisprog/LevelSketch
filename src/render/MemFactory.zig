const stb = @import("stb");
const std = @import("std");
const zbgfx = @import("zbgfx");

pub const Context = enum {
    stb_image,
    buffer,
    static_buffer,
};

/// This object handles freeing memory allocations that have been uploaded to the GPU.
/// 'bgfx' provides a callback for when the upload is complete, but it is invoked on the
/// render thread. Need to marshall this info back to the main thread to free the memory.
const Self = @This();

allocator: std.mem.Allocator,
_allocations: std.ArrayList(*Allocation),
_id: u32 = 1,
_mutex: std.Thread.Mutex = .{},

pub fn init(allocator: std.mem.Allocator) !Self {
    const allocations = try std.ArrayList(*Allocation).initCapacity(allocator, 0);
    return Self{
        .allocator = allocator,
        ._allocations = allocations,
    };
}

pub fn deinit(self: *Self) void {
    for (self._allocations.items) |alloc| {
        alloc.free(self.allocator);
        self.allocator.destroy(alloc);
    }
    self._allocations.deinit(self.allocator);
}

pub fn create(self: *Self, data: []const u8, context: Context) ![*c]const zbgfx.bgfx.Memory {
    switch (context) {
        .stb_image, .buffer => {
            const allocation = try self.addAllocation(data, context);
            const mem = zbgfx.bgfx.makeRefRelease(
                data.ptr,
                @intCast(data.len),
                @ptrCast(@constCast(&onReleaseRef)),
                @ptrCast(allocation),
            );
            return mem;
        },
        .static_buffer => {
            const mem = zbgfx.bgfx.makeRef(data.ptr, @intCast(data.len));
            return mem;
        },
    }
}

pub fn update(self: *Self) void {
    if (self._allocations.items.len == 0) {
        return;
    }

    if (!self._mutex.tryLock()) {
        return;
    }
    defer self._mutex.unlock();

    var i: usize = 0;
    while (i < self._allocations.items.len) {
        if (self._allocations.items[i].state == .complete) {
            const alloc = self._allocations.swapRemove(i);

            alloc.free(self.allocator);
            self.allocator.destroy(alloc);
        } else {
            i += 1;
        }
    }
}

fn addAllocation(self: *Self, data: []const u8, context: Context) !*Allocation {
    self._mutex.lock();
    defer self._mutex.unlock();

    const item = try self.allocator.create(Allocation);
    item.*.id = self._id;
    item.*.data = data;
    item.*.context = context;
    self._id += 1;

    try self._allocations.append(self.allocator, item);
    return self._allocations.getLast();
}

/// Will be called on the render thread.
fn onReleaseRef(ptr: ?*anyopaque, context: ?*anyopaque) callconv(.c) void {
    _ = ptr;
    const handler: *Allocation = @ptrCast(@alignCast(context.?));
    handler.*.state = .complete;
}

const Allocation = struct {
    pub const State = enum {
        loading,
        complete,
    };

    id: u32,
    context: Context,
    data: []const u8,
    state: State = .loading,

    fn free(self: Allocation, allocator: std.mem.Allocator) void {
        switch (self.context) {
            .stb_image => {
                stb.image.free_data(self.data);
            },
            .buffer => {
                allocator.free(self.data);
            },
            .static_buffer => {},
        }
    }
};
