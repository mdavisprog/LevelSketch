const Components = @import("Components.zig");
const core = @import("core");
const std = @import("std");
const world = @import("root.zig");

const Signature = Components.Signature;
const HashSetUnmanaged = core.containers.HashSetUnmanaged;
const Entity = world.Entity;
const World = world.World;

/// Holds all registered systems and updates what entities a system should run on.
const Self = @This();

/// Id representing a registered system. A value of '0' represents an invalid system.
pub const SystemId = u32;

/// The list of entities that is part of a system. Also includes the world object for
/// component access.
pub const SystemParam = struct {
    world: *World,
    allocator: std.mem.Allocator,

    pub fn getComponent(self: SystemParam, comptime T: type, entity: Entity) ?*T {
        return self.world.getComponent(T, entity);
    }
};

pub const Schedule = enum {
    startup,
    update,
    render,
    shutdown,
};
const schedule_count = @typeInfo(Schedule).@"enum".fields.len;
pub const invalid_system: SystemId = 0;

systems: [schedule_count]std.AutoArrayHashMapUnmanaged(SystemId, ISystem) = @splat(.empty),
_system_id: SystemId = 1,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    for (&self.systems) |*systems| {
        var it = systems.iterator();
        while (it.next()) |entry| {
            entry.value_ptr.deinit(allocator);
        }
        systems.deinit(allocator);
    }
}

pub fn register(
    self: *Self,
    allocator: std.mem.Allocator,
    comptime system: anytype,
    params: *ParamsType(system),
    schedule: Schedule,
) !SystemId {
    const id = self._system_id;
    var systems = &self.systems[@intFromEnum(schedule)];
    try systems.put(allocator, id, try generateSystem(system, allocator, params));
    self._system_id += 1;

    return id;
}

pub fn unregister(self: *Self, allocator: std.mem.Allocator, system: SystemId) void {
    for (&self.systems) |*systems| {
        const entry = systems.getPtr(system) orelse continue;
        entry.deinit(allocator);
        _ = systems.orderedRemove(system);
        break;
    }
}

pub fn run(self: *Self, schedule: Schedule) void {
    const systems = &self.systems[@intFromEnum(schedule)];
    var it = systems.iterator();
    while (it.next()) |entry| {
        const system = entry.value_ptr;
        system.invoke() catch |err| {
            std.debug.panic("Failed to run system: '{s}'. Error: {}.", .{
                system.getName(),
                err,
            });
        };
    }
}

const ISystem = struct {
    const VTable = struct {
        deinit: *const fn (ptr: *anyopaque, allocator: std.mem.Allocator) void,
        invoke: *const fn (ptr: *anyopaque) anyerror!void,
        getName: *const fn () []const u8,
    };

    ptr: *anyopaque,
    vtable: VTable,

    fn deinit(self: ISystem, allocator: std.mem.Allocator) void {
        self.vtable.deinit(self.ptr, allocator);
    }

    fn invoke(self: ISystem) !void {
        try self.vtable.invoke(self.ptr);
    }

    fn getName(self: ISystem) []const u8 {
        return self.vtable.getName();
    }
};

fn ParamsType(comptime system: anytype) type {
    return std.meta.ArgsTuple(@TypeOf(system));
}

fn generateSystem(
    comptime system: anytype,
    allocator: std.mem.Allocator,
    params: *ParamsType(system),
) !ISystem {
    const System = struct {
        const SystemSelf = @This();
        const Type = @TypeOf(system);

        func: *const Type,
        params: *ParamsType(system),

        fn deinit(ptr: *anyopaque, _allocator: std.mem.Allocator) void {
            const self: *SystemSelf = @ptrCast(@alignCast(ptr));
            _allocator.destroy(self.params);
            _allocator.destroy(self);
        }

        fn invoke(ptr: *anyopaque) !void {
            const self: *SystemSelf = @ptrCast(@alignCast(ptr));
            try @call(.auto, self.func, self.params.*);
        }

        fn getName() []const u8 {
            return @typeName(Type);
        }
    };

    const ptr = try allocator.create(System);
    ptr.func = system;
    ptr.params = params;
    return .{
        .ptr = ptr,
        .vtable = .{
            .deinit = System.deinit,
            .invoke = System.invoke,
            .getName = System.getName,
        },
    };
}
