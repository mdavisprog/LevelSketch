pub const image = @import("image.zig");

const std = @import("std");

pub fn init(allocator: std.mem.Allocator) !void {
    allocations = try Allocations.init(allocator);

    zMallocPtr = zMalloc;
    zReallocPtr = zRealloc;
    zFreePtr = zFree;
}

pub fn deinit() void {
    if (allocations) |*allocs| {
        allocs.deinit();
    }
}

const Allocations = struct {
    const Self = @This();

    const Allocation = struct {
        address: usize,
        size: usize,
    };

    allocs: std.ArrayList(Allocation),
    allocator: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator) !Self {
        return Self{
            .allocs = try std.ArrayList(Allocation).initCapacity(allocator, 0),
            .allocator = allocator,
        };
    }

    pub fn deinit(self: *Self) void {
        self.allocs.deinit(self.allocator);
    }

    pub fn malloc(self: *Self, size: usize) []u8 {
        const result = self.allocator.alloc(u8, size) catch |err| {
            std.debug.panic("Failed to allocate memory: {}", .{err});
        };
        self.add(result);
        return result;
    }

    pub fn realloc(self: *Self, alloc: ?*anyopaque, size: usize) []u8 {
        const old_size = if (alloc) |p| self.remove(p) else 0;
        const ptr: [*]u8 = if (alloc) |p| @ptrCast(p) else undefined;
        const result = self.allocator.realloc(ptr[0..old_size], size) catch |err| {
            std.debug.panic("Failed to realloc memory: {}", .{err});
        };
        self.add(result);
        return result;
    }

    pub fn free(self: *Self, alloc: *anyopaque) void {
        const size = self.remove(alloc);
        const ptr: [*]u8 = @ptrCast(alloc);
        self.allocator.free(ptr[0..size]);
    }

    fn numAllocs(self: Self) usize {
        return self.allocs.items.len;
    }

    fn add(self: *Self, alloc: []u8) void {
        self.allocs.append(self.allocator, .{
            .address = @intFromPtr(alloc.ptr),
            .size = alloc.len,
        }) catch |err| {
            std.debug.panic("Failed to add allocation: {}", .{err});
        };
    }

    fn remove(self: *Self, alloc: *anyopaque) usize {
        for (self.allocs.items, 0..) |item, i| {
            if (item.address == @intFromPtr(alloc)) {
                const removed = self.allocs.swapRemove(i);
                return removed.size;
            }
        }

        return 0;
    }

    fn get(self: Self, alloc: *anyopaque) usize {
        for (self.allocs.items) |item| {
            if (item.address == @intFromPtr(alloc)) {
                return item.size;
            }
        }

        return 0;
    }
};

var allocations: ?Allocations = null;

extern var zMallocPtr: ?*const fn (size: usize) callconv(.c) ?*anyopaque;
extern var zReallocPtr: ?*const fn (ptr: ?*anyopaque, size: usize) callconv(.c) ?*anyopaque;
extern var zFreePtr: ?*const fn (ptr: ?*anyopaque) callconv(.c) void;

fn zMalloc(size: usize) callconv(.c) ?*anyopaque {
    verify();
    const result = allocations.?.malloc(size);
    return result.ptr;
}

fn zRealloc(ptr: ?*anyopaque, size: usize) callconv(.c) ?*anyopaque {
    if (size == 0) {
        @panic("Invalid size");
    }

    verify();
    const result = allocations.?.realloc(ptr, size);
    return result.ptr;
}

fn zFree(ptr: ?*anyopaque) callconv(.c) void {
    if (ptr == null) {
        return;
    }

    verify();
    allocations.?.free(ptr.?);
}

fn verify() void {
    if (allocations == null) {
        @panic("stb module not initialized");
    }
}
