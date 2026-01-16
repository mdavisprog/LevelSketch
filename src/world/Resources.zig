const std = @import("std");

/// Holds all individual resources. Resources are objects that contain information which can
/// be shared across all systems.
const Self = @This();
const Map = std.StringHashMapUnmanaged(IResource);

_map: Map = .empty,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    var it = self._map.valueIterator();
    while (it.next()) |value| {
        value.vtable.deinit(value.ptr, allocator);
    }
    self._map.deinit(allocator);
}

pub fn add(self: *Self, comptime T: type, allocator: std.mem.Allocator, resource: T) !void {
    const item = try allocator.create(T);
    item.* = resource;
    try self._map.put(allocator, @typeName(T), generateResource(T, item));
}

pub fn get(self: Self, comptime T: type) ?*T {
    const result = self._map.get(@typeName(T)) orelse return null;
    return @ptrCast(@alignCast(result.ptr));
}

const IResource = struct {
    const VTable = struct {
        deinit: *const fn (ptr: *anyopaque, allocator: std.mem.Allocator) void,
    };

    ptr: *anyopaque,
    vtable: VTable,
};

fn generateResource(comptime T: type, ptr: *T) IResource {
    const Resource = struct {
        fn deinit(_ptr: *anyopaque, allocator: std.mem.Allocator) void {
            const resource: *T = @ptrCast(@alignCast(_ptr));
            allocator.destroy(resource);
        }
    };

    return .{
        .ptr = ptr,
        .vtable = .{
            .deinit = Resource.deinit,
        },
    };
}
