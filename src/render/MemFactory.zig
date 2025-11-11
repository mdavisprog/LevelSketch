const std = @import("std");
const zbgfx = @import("zbgfx");

pub const Mem = struct {
    ptr: [*c]const zbgfx.bgfx.Memory,
};

pub const OnUploadedResult = struct {
    allocator: std.mem.Allocator,
    data: []const u8,
    user_data: ?*anyopaque,

    pub fn userDataAs(self: OnUploadedResult, comptime T: type) *T {
        return @ptrCast(@alignCast(self.user_data));
    }
};

pub const OnUploaded = *const fn (result: OnUploadedResult) void;

/// This object create bgfx.Memory objects and allows callers to provide a callback to listen
/// for when the data has been uploaded to the GPU. This object ensures that the callback is
/// invoked on the main thread. The caller is responsible for the memory after upload.
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

/// Callback can be null if caller wishes to ignore the upload status. The caller must ensure that
/// the memory is valid during GPU upload.
pub fn create(
    self: *Self,
    data: []const u8,
    callback: ?OnUploaded,
    user_data: ?*anyopaque,
) !Mem {
    const mem = blk: {
        if (callback == null) {
            // The user owns the memory and does not care about the upload state.
            const mem = zbgfx.bgfx.makeRef(data.ptr, @intCast(data.len));
            break :blk mem;
        } else {
            const allocation = try self.addAllocation(data);
            allocation.*.callback = callback;
            allocation.*.user_data = user_data;

            const mem = zbgfx.bgfx.makeRefRelease(
                data.ptr,
                @intCast(data.len),
                @ptrCast(@constCast(&onReleaseRef)),
                @ptrCast(allocation),
            );

            break :blk mem;
        }
    };

    return Mem{
        .ptr = mem,
    };
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

fn addAllocation(self: *Self, data: []const u8) !*Allocation {
    self._mutex.lock();
    defer self._mutex.unlock();

    const item = try self.allocator.create(Allocation);
    item.*.id = self._id;
    item.*.data = data;
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
    data: []const u8,
    state: State = .loading,
    callback: ?OnUploaded = null,
    user_data: ?*anyopaque = null,

    fn free(self: Allocation, allocator: std.mem.Allocator) void {
        if (self.callback) |callback| {
            callback(.{
                .allocator = allocator,
                .data = self.data,
                .user_data = self.user_data,
            });
        }
    }
};
