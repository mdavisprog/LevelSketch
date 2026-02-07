const hash = @import("hash.zig");
const std = @import("std");
const world = @import("root.zig");

const SystemId = world.Systems.SystemId;

/// Systems that can be triggered through an event.
const Self = @This();

pub const Event = u32;

items: std.AutoHashMapUnmanaged(Event, std.ArrayListUnmanaged(SystemId)) = .empty,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    var it = self.items.valueIterator();
    while (it.next()) |systems| {
        systems.deinit(allocator);
    }
    self.items.deinit(allocator);
}

pub fn add(self: *Self, allocator: std.mem.Allocator, comptime T: type) !void {
    const id = hash.hashStruct(T);

    if (self.items.contains(id)) {
        std.debug.panic("Event {s} with id {} already exists!", .{ @typeName(T), id });
    }

    try self.items.put(allocator, id, .empty);
}

pub fn addListener(
    self: *Self,
    comptime T: type,
    allocator: std.mem.Allocator,
    system: SystemId,
) !void {
    const id = hash.hashStruct(T);
    const events = self.items.getPtr(id) orelse {
        std.debug.panic(
            "Failed to add listener. Event of type '{s}' has not been registered.",
            .{@typeName(T)},
        );
    };
    try events.append(allocator, system);
}

pub fn contains(self: Self, comptime T: type) bool {
    const id = hash.hashStruct(T);
    return self.items.contains(id);
}

pub fn getListeners(self: Self, comptime T: type) []const SystemId {
    const id = hash.hashStruct(T);
    const systems = self.items.get(id) orelse {
        std.debug.panic(
            "Failed to get systems. Event of type '{s}' not registered.",
            .{@typeName(T)},
        );
    };
    return systems.items;
}
