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

pub const Query = struct {
    pub const Result = struct {
        entities: HashSetUnmanaged(Entity) = .empty,
    };

    components: []const type,

    pub fn init(comptime components: []const type) Query {
        return .{
            .components = components,
        };
    }
};

/// Id representing a registered system. A value of '0' represents an invalid system.
pub const SystemId = u32;

/// The list of entities that is part of a system. Also includes the world object for
/// component access.
pub const SystemParam = struct {
    queries: []const Query.Result,
    world: *World,
    allocator: std.mem.Allocator,

    pub fn getEntities(self: SystemParam, query: usize) HashSetUnmanaged(Entity).Iterator {
        return self.queries[query].entities.iterator();
    }

    pub fn getComponent(self: SystemParam, comptime T: type, entity: Entity) ?*T {
        return self.world.getComponent(T, entity);
    }
};

/// Function signature for systems with direct world access.
pub const WorldSystem = *const fn (param: SystemParam) anyerror!void;

pub const Schedule = enum {
    startup,
    update,
    render,
    shutdown,
};
const schedule_count = @typeInfo(Schedule).@"enum".fields.len;
pub const invalid_system: SystemId = 0;

systems: [schedule_count]std.AutoArrayHashMapUnmanaged(SystemId, SystemEntry) = @splat(.empty),
_signatures: std.AutoHashMapUnmanaged(SystemId, SystemQueries) = .empty,
_system_id: SystemId = 1,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    for (&self.systems) |*systems| {
        var it = systems.iterator();
        while (it.next()) |entry| {
            for (entry.value_ptr.queries) |*query| {
                query.entities.deinit(allocator);
            }
            allocator.free(entry.value_ptr.queries);
        }

        systems.deinit(allocator);
    }

    var signatures = self._signatures.valueIterator();
    while (signatures.next()) |signature| {
        allocator.free(signature.queries);
    }
    self._signatures.deinit(allocator);
}

pub fn register(
    self: *Self,
    allocator: std.mem.Allocator,
    schedule: Schedule,
    system: WorldSystem,
    num_queries: usize,
) !SystemId {
    var systems = &self.systems[@intFromEnum(schedule)];
    const id = self._system_id;
    try systems.put(allocator, id, .{
        .system = system,
        .queries = try allocator.alloc(Query.Result, num_queries),
        .run_condition = if (num_queries == 0) .always else .entities,
    });
    self._system_id += 1;

    var it = systems.iterator();
    while (it.next()) |entry| {
        for (entry.value_ptr.queries) |*query| {
            query.* = .{};
        }
    }

    const signatures = try self._signatures.getOrPut(allocator, id);
    signatures.value_ptr.queries = try allocator.alloc(Signature, num_queries);
    for (signatures.value_ptr.queries) |*query| {
        query.* = .initEmpty();
    }

    return id;
}

pub fn unregister(self: *Self, allocator: std.mem.Allocator, system: SystemId) void {
    for (&self.systems) |*systems| {
        const entry = systems.getPtr(system) orelse continue;
        for (entry.queries) |*query| {
            query.entities.deinit(allocator);
        }
        allocator.free(entry.queries);
        _ = systems.orderedRemove(system);
        break;
    }

    if (self._signatures.getPtr(system)) |signatures| {
        allocator.free(signatures.queries);
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
            .entities => blk: {
                for (item.queries) |query| {
                    if (!query.entities.isEmpty()) {
                        break :blk true;
                    }
                }
                break :blk false;
            },
        };

        if (should_run) {
            item.system(.{
                .queries = entry.value_ptr.queries,
                .world = _world,
                .allocator = _world._allocator,
            }) catch |err| {
                std.debug.panic("Failed to run system: '{}'. Error: {}.", .{
                    item.system,
                    err,
                });
            };
        }
    }
}

pub fn setSignature(
    self: *Self,
    allocator: std.mem.Allocator,
    query_index: usize,
    system: SystemId,
    signature: Signature,
) !void {
    const entry = try self._signatures.getOrPut(allocator, system);
    entry.value_ptr.queries[query_index] = signature;
}

pub fn entityDestroyed(self: *Self, entity: Entity) void {
    for (&self.systems) |systems| {
        var it = systems.iterator();
        while (it.next()) |entry| {
            for (entry.value_ptr.queries) |*query| {
                _ = query.entities.remove(entity);
            }
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
            const system_signatures = self._signatures.get(id) orelse continue;
            for (system_signatures.queries, 0..) |query, i| {
                const intersection = query.intersectWith(signature);

                if (intersection.eql(query)) {
                    try entry.value_ptr.queries[i].entities.insert(allocator, entity);
                } else {
                    _ = entry.value_ptr.queries[i].entities.remove(entity);
                }
            }
        }
    }
}

pub fn hasQuery(self: Self, system: SystemId, query: Signature) bool {
    const signature = self._signatures.get(system) orelse return false;
    for (signature.queries) |_query| {
        if (_query.eql(query)) {
            return true;
        }
    }

    return false;
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
    queries: []Query.Result,
    run_condition: RunCondition = .entities,
};

const SystemQueries = struct {
    queries: []Signature,
};
