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
    entities: HashSetUnmanaged(Entity) = .empty,
    world: *World,
};

/// Function signature for systems with direct world access.
pub const WorldSystem = *const fn (param: SystemParam) void;

pub const Schedule = enum {
    startup,
    update,
    render,
    shutdown,
};
const schedule_count = @typeInfo(Schedule).@"enum".fields.len;
pub const invalid_system: SystemId = 0;

systems: [schedule_count]std.AutoArrayHashMapUnmanaged(SystemId, SystemEntry) = @splat(.empty),
_signatures: std.AutoHashMapUnmanaged(SystemId, Signature) = .empty,
_system_id: SystemId = 1,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    for (&self.systems) |*systems| {
        var it = systems.iterator();
        while (it.next()) |entry| {
            entry.value_ptr.entities.deinit(allocator);
        }

        systems.deinit(allocator);
    }

    self._signatures.deinit(allocator);
}

pub fn register(
    self: *Self,
    allocator: std.mem.Allocator,
    schedule: Schedule,
    system: WorldSystem,
    always_run: bool,
) !SystemId {
    var systems = &self.systems[@intFromEnum(schedule)];
    const id = self._system_id;
    try systems.put(allocator, id, .{
        .system = system,
        .run_condition = if (always_run) .always else .entities,
    });
    self._system_id += 1;
    return id;
}

pub fn unregister(self: *Self, allocator: std.mem.Allocator, system: SystemId) void {
    for (&self.systems) |*systems| {
        const entry = systems.getPtr(system) orelse continue;
        entry.entities.deinit(allocator);
        _ = systems.orderedRemove(system);
        break;
    }

    _ = self._signatures.remove(system);
}

pub fn run(self: *Self, schedule: Schedule, _world: *World) void {
    const systems = self.systems[@intFromEnum(schedule)];
    var it = systems.iterator();
    while (it.next()) |entry| {
        const item = entry.value_ptr.*;

        const should_run = switch (item.run_condition) {
            .always => true,
            .entities => !item.entities.isEmpty(),
        };

        if (should_run) {
            item.system(.{
                .entities = entry.value_ptr.entities,
                .world = _world,
            });
        }
    }
}

pub fn setSignature(
    self: *Self,
    allocator: std.mem.Allocator,
    system: SystemId,
    signature: Signature,
) !void {
    try self._signatures.put(allocator, system, signature);
}

pub fn entityDestroyed(self: *Self, entity: Entity) void {
    for (&self.systems) |systems| {
        var it = systems.iterator();
        while (it.next()) |entry| {
            _ = entry.value_ptr.entities.remove(entity);
        }
    }
}

pub fn entitySignatureChanged(
    self: *Self,
    allocator: std.mem.Allocator,
    entity: Entity,
    signature: Signature,
) !void {
    for (&self.systems) |systems| {
        var it = systems.iterator();
        while (it.next()) |entry| {
            const id = entry.key_ptr.*;
            const system_signature = self._signatures.get(id) orelse continue;
            const intersection = system_signature.intersectWith(signature);

            if (intersection.eql(system_signature)) {
                try entry.value_ptr.entities.insert(allocator, entity);
            } else {
                _ = entry.value_ptr.entities.remove(entity);
            }
        }
    }
}

/// Determines if a system should be run.
const RunCondition = enum {
    // Will always run regardless if entities are registered with this system or not.
    always,

    // Only runs if entities are available.
    entities,
};

const SystemEntry = struct {
    system: WorldSystem,
    entities: HashSetUnmanaged(Entity) = .empty,
    run_condition: RunCondition = .entities,
};
