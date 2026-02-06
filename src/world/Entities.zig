const Components = @import("Components.zig");
const std = @import("std");
const world = @import("root.zig");

const Signature = Components.Signature;

pub const Entity = struct {
    pub const Id = u32;

    id: Id = 0,
};

const Self = @This();

const EntityList = std.ArrayListUnmanaged(Entity.Id);

pub const max: Entity.Id = 5000;

deleted: EntityList = .empty,
signatures: [max]Signature = @splat(.initEmpty()),
count: usize = 0,
_id: Entity.Id = 0,

pub fn init(allocator: std.mem.Allocator) Self {
    _ = allocator;
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self.deleted.deinit(allocator);
}

pub fn create(self: *Self) Entity {
    const id = blk: {
        if (self.deleted.pop()) |id| {
            break :blk id;
        } else {
            const id = self._id;
            self._id += 1;
            break :blk id;
        }
    };

    std.debug.assert(self._id < max);
    self.count += 1;

    return .{
        .id = id,
    };
}

pub fn destroy(self: *Self, allocator: std.mem.Allocator, entity: Entity) !void {
    try self.deleted.append(allocator, entity.id);

    self.signatures[entity.id] = .initEmpty();
    self.count -= 1;
}

pub fn setSignatureBit(self: *Self, entity: Entity, index: usize) void {
    self.signatures[entity.id].set(index);
}

pub fn unsetSignatureBit(self: *Self, entity: Entity, index: usize) void {
    self.signatures[entity.id].unset(index);
}

pub fn getSignature(self: Self, entity: Entity) Signature {
    return self.signatures[entity.id];
}
