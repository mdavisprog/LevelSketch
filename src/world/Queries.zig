const Components = @import("Components.zig");
const std = @import("std");
const world = @import("root.zig");

const Entity = world.Entity;
const HashSetUnmanaged = world.HashSetUnmanaged;
const Signature = Components.Signature;

pub const EntitySet = HashSetUnmanaged(Entity);

/// Generate a query for a system.
pub fn Query(comptime components: []const type) type {
    return struct {
        const QuerySelf = @This();

        comptime components: []const type = components,
        entities: ?*const EntitySet = null,

        pub fn getEntities(self: QuerySelf) EntitySet.Iterator {
            const entities = self.entities orelse {
                std.debug.panic(
                    "Query '{s}' does not have a valid 'entities' set.",
                    .{@typeName(QuerySelf)},
                );
            };
            return entities.iterator();
        }

        pub fn numEntities(self: QuerySelf) usize {
            const entities = self.entities orelse return 0;
            return entities.count();
        }
    };
}

pub inline fn isQueryType(comptime query: type) bool {
    const info = @typeInfo(query);
    if (info != .@"struct") {
        return false;
    }

    const struct_info = info.@"struct";
    if (struct_info.fields.len != 2) {
        return false;
    }

    const components = struct_info.fields[0];
    if (!components.is_comptime) {
        return false;
    }

    if (components.type != []const type) {
        return false;
    }

    const entities = struct_info.fields[1];
    if (entities.type != ?*const HashSetUnmanaged(Entity)) {
        return false;
    }

    return true;
}

/// Holds the list of entities per query that can be used for Systems, Events, etc.
const Self = @This();

entries: std.AutoHashMapUnmanaged(Signature, IQuery) = .empty,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    var it = self.entries.valueIterator();
    while (it.next()) |query| {
        query.entities.*.deinit(allocator);
        allocator.destroy(query.entities);
        query.vtable.deinit(query.ptr, allocator);
    }
    self.entries.deinit(allocator);
}

pub fn add(
    self: *Self,
    comptime QueryType: type,
    allocator: std.mem.Allocator,
    signature: Signature,
) !*const EntitySet {
    if (self.entries.contains(signature)) {
        return self.entries.getPtr(signature).?.entities;
    }

    const entities = try allocator.create(EntitySet);
    errdefer allocator.destroy(entities);
    entities.* = .empty;

    const query = try allocator.create(QueryType);
    errdefer allocator.destroy(query);
    query.* = .{
        .entities = entities,
    };

    try self.entries.put(allocator, signature, generateIQuery(QueryType, query, entities));

    return entities;
}

pub fn get(self: Self, signature: Signature) ?*anyopaque {
    const query = self.entries.get(signature) orelse return null;
    return query.ptr;
}

pub fn signatureChanged(
    self: *Self,
    allocator: std.mem.Allocator,
    signature: Signature,
    entity: Entity,
) !void {
    var it = self.entries.iterator();
    while (it.next()) |entry| {
        const query = entry.key_ptr.*;
        const intersection = query.intersectWith(signature);

        if (intersection.eql(query)) {
            try entry.value_ptr.entities.insert(allocator, entity);
        } else {
            _ = entry.value_ptr.entities.remove(entity);
        }
    }
}

pub fn entityDestroyed(self: *Self, entity: Entity) void {
    var it = self.entries.valueIterator();
    while (it.next()) |query| {
        _ = query.entities.remove(entity);
    }
}

const IQuery = struct {
    const VTable = struct {
        deinit: *const fn (ptr: *anyopaque, allocator: std.mem.Allocator) void,
    };

    ptr: *anyopaque,
    entities: *EntitySet,
    vtable: VTable,
};

fn generateIQuery(comptime T: type, ptr: *anyopaque, entities: *EntitySet) IQuery {
    const Bindings = struct {
        fn deinit(_ptr: *anyopaque, allocator: std.mem.Allocator) void {
            const query: *T = @ptrCast(@alignCast(_ptr));
            allocator.destroy(query);
        }
    };

    return .{
        .ptr = ptr,
        .entities = entities,
        .vtable = .{
            .deinit = Bindings.deinit,
        },
    };
}
