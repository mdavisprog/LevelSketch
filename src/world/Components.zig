const Entities = @import("Entities.zig");
const std = @import("std");
const world = @import("root.zig");

const Entity = world.Entity;

/// Holds all arrays for each registered component type.
const Self = @This();

pub const Id = u32;

const Arrays = std.AutoHashMapUnmanaged(Id, IArray);
const Types = std.StringHashMapUnmanaged(Id);

pub const max: usize = 512;
pub const Signature = std.StaticBitSet(max);

_id: Id = 1,
_arrays: Arrays = .empty,
_types: Types = .empty,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    var it = self._arrays.valueIterator();
    while (it.next()) |array| {
        array.deinit_fn(array.ptr, allocator);
    }
    self._arrays.deinit(allocator);
    self._types.deinit(allocator);
}

pub fn register(self: *Self, comptime T: type, allocator: std.mem.Allocator) !void {
    const id = self._id;
    var array: *Array(T) = try .init(allocator);
    try self._arrays.put(allocator, id, array.interface());
    try self._types.put(allocator, @typeName(T), id);
    self._id += 1;
}

pub fn insert(
    self: *Self,
    comptime T: type,
    allocator: std.mem.Allocator,
    entity: Entity,
    component: T,
) !void {
    const key = @typeName(T);
    const type_id = self._types.get(key) orelse return;
    const array = self._arrays.get(type_id) orelse return;
    try array.insert_fn(array.ptr, allocator, entity, @constCast(&component));
}

pub fn remove(self: *Self, comptime T: type, allocator: std.mem.Allocator, entity: Entity) !void {
    const key = @typeName(T);
    const type_id = self._types.get(key) orelse return;
    const array = self._arrays.get(type_id) orelse return;
    try array.remove_fn(array.ptr, allocator, entity);
}

pub fn entityDestroyed(self: Self, allocator: std.mem.Allocator, entity: Entity) !void {
    var it = self._arrays.valueIterator();
    while (it.next()) |array| {
        try array.entity_destroyed_fn(array.ptr, allocator, entity);
    }
}

pub fn get(self: Self, comptime T: type, entity: Entity) ?*T {
    const key = @typeName(T);
    const type_id = self._types.get(key) orelse return null;
    const array = self._arrays.get(type_id) orelse return null;
    const result = array.get_fn(array.ptr, entity) orelse return null;
    return @ptrCast(@alignCast(result));
}

pub fn getComponentId(self: Self, comptime T: type) ?Id {
    const key = @typeName(T);
    return self._types.get(key);
}

pub fn getComponentIdByName(self: Self, component: []const u8) ?Id {
    return self._types.get(component);
}

const IArray = struct {
    const DeinitFn = *const fn (ptr: *anyopaque, allocator: std.mem.Allocator) void;
    const InsertFn = *const fn (
        ptr: *anyopaque,
        allocator: std.mem.Allocator,
        entity: Entity,
        component: *anyopaque,
    ) anyerror!void;
    const RemoveFn = *const fn (
        ptr: *anyopaque,
        allocator: std.mem.Allocator,
        entity: Entity,
    ) anyerror!void;
    const EntityDestroyedFn = *const fn (
        ptr: *anyopaque,
        allocator: std.mem.Allocator,
        entity: Entity,
    ) anyerror!void;
    const GetFn = *const fn (ptr: *anyopaque, entity: Entity) ?*anyopaque;

    ptr: *anyopaque,
    deinit_fn: DeinitFn,
    insert_fn: InsertFn,
    remove_fn: RemoveFn,
    entity_destroyed_fn: EntityDestroyedFn,
    get_fn: GetFn,
};

/// Holds all components for a specific component type.
fn Array(comptime T: type) type {
    return struct {
        const ArraySelf = @This();

        pub const empty: ArraySelf = .{};

        components: std.ArrayListUnmanaged(T) = .empty,
        entity_to_index: std.AutoHashMapUnmanaged(Entity.Id, usize) = .empty,
        index_to_entity: std.AutoHashMapUnmanaged(usize, Entity.Id) = .empty,

        pub fn init(allocator: std.mem.Allocator) !*ArraySelf {
            const result = try allocator.create(ArraySelf);
            result.* = .{
                .components = try .initCapacity(allocator, 255),
            };
            return result;
        }

        pub fn deinit(self: *ArraySelf, allocator: std.mem.Allocator) void {
            self.components.deinit(allocator);
            self.entity_to_index.deinit(allocator);
            self.index_to_entity.deinit(allocator);
            allocator.destroy(self);
        }

        pub fn insert(
            self: *ArraySelf,
            allocator: std.mem.Allocator,
            entity: Entity,
            component: T,
        ) !void {
            std.debug.assert(!self.entity_to_index.contains(entity.id));

            const index = self.components.items.len;

            try self.entity_to_index.put(allocator, entity.id, index);
            try self.index_to_entity.put(allocator, index, entity.id);

            try self.components.append(allocator, component);
        }

        pub fn remove(self: *ArraySelf, allocator: std.mem.Allocator, entity: Entity) !void {
            std.debug.assert(self.entity_to_index.contains(entity.id));

            const removed_index = self.entity_to_index.get(entity.id) orelse {
                std.debug.panic("Failed to retrieve component index for entity: {}.", .{
                    entity.id,
                });
            };
            const last_index = self.components.items.len - 1;
            _ = self.components.swapRemove(removed_index);

            const last_entity = self.index_to_entity.get(last_index) orelse {
                std.debug.panic("Failed to entity id for component index: {}.", .{
                    last_index,
                });
            };
            try self.entity_to_index.put(allocator, last_entity, removed_index);
            try self.index_to_entity.put(allocator, removed_index, last_entity);

            _ = self.entity_to_index.remove(entity.id);
            _ = self.index_to_entity.remove(last_index);
        }

        pub fn entityDestroyed(self: *ArraySelf, allocator: std.mem.Allocator, entity: Entity) !void {
            if (self.entity_to_index.contains(entity.id)) {
                try self.remove(allocator, entity);
            }
        }

        pub fn getMut(self: *ArraySelf, entity: Entity) ?*T {
            const index = self.entity_to_index.get(entity.id) orelse return null;
            return &self.components.items[index];
        }

        fn onDeinit(ptr: *anyopaque, allocator: std.mem.Allocator) void {
            const self = fromPtr(ptr);
            self.deinit(allocator);
        }

        fn onInsert(
            ptr: *anyopaque,
            allocator: std.mem.Allocator,
            entity: Entity,
            component: *anyopaque,
        ) !void {
            const self = fromPtr(ptr);
            const _component: *T = @ptrCast(@alignCast(component));
            try self.insert(allocator, entity, _component.*);
        }

        fn onRemove(
            ptr: *anyopaque,
            allocator: std.mem.Allocator,
            entity: Entity,
        ) !void {
            const self = fromPtr(ptr);
            try self.remove(allocator, entity);
        }

        fn onEntityDestroyed(ptr: *anyopaque, allocator: std.mem.Allocator, entity: Entity) !void {
            const self = fromPtr(ptr);
            try self.entityDestroyed(allocator, entity);
        }

        fn onGet(ptr: *anyopaque, entity: Entity) ?*anyopaque {
            const self = fromPtr(ptr);
            return self.getMut(entity);
        }

        fn fromPtr(ptr: *anyopaque) *ArraySelf {
            return @ptrCast(@alignCast(ptr));
        }

        fn interface(self: *ArraySelf) IArray {
            return .{
                .ptr = self,
                .deinit_fn = onDeinit,
                .insert_fn = onInsert,
                .remove_fn = onRemove,
                .entity_destroyed_fn = onEntityDestroyed,
                .get_fn = onGet,
            };
        }
    };
}
