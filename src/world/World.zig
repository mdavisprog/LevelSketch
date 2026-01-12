const Components = @import("Components.zig");
const Entities = @import("Entities.zig");
const std = @import("std");
const world = @import("root.zig");

const Signature = Components.Signature;
const Camera = world.Camera;
const Entity = world.Entity;
const Systems = world.Systems;
const SystemParam = Systems.SystemParam;
const World = world.World;

const Self = @This();

camera: Camera = .{},
light_orbit: bool = true,
entities: Entities,
components: Components,
systems: Systems,
_allocator: std.mem.Allocator,

pub fn init(allocator: std.mem.Allocator) !Self {
    var components: Components = .init();
    try components.register(world.components.Transform, allocator);

    return .{
        .entities = .init(allocator),
        .components = components,
        .systems = .init(),
        ._allocator = allocator,
    };
}

pub fn deinit(self: *Self) void {
    self.entities.deinit(self._allocator);
    self.components.deinit(self._allocator);
    self.systems.deinit(self._allocator);
}

pub fn createEntity(self: *Self) Entity {
    return self.entities.create();
}

pub fn destroyEntity(self: *Self, entity: Entity) !void {
    try self.entities.destroy(self._allocator, entity);
    try self.components.entityDestroyed(self._allocator, entity);
    self.systems.entityDestroyed(entity);
}

pub fn registerComponent(self: *Self, comptime T: type) !void {
    try self.components.register(T, self._allocator);
}

pub fn registerComponents(self: *Self, comptime components: []const type)  !void {
    inline for (components) |component| {
        try self.components.register(component, self._allocator);
    }
}

pub fn insertComponent(self: *Self, comptime T: type, entity: Entity, component: T) !void {
    // Add the actual component data
    try self.components.insert(T, self._allocator, entity, component);

    // Update which components the given entity has
    const component_id = self.components.getComponentId(T) orelse return;
    self.entities.setSignatureBit(entity, @intCast(component_id));

    // Notify all systems to move the entity to the proper sets
    const signature = self.entities.getSignature(entity);
    try self.systems.entitySignatureChanged(self._allocator, entity, signature);
}

pub fn removeComponent(self: *Self, comptime T: type, entity: Entity) !void {
    // Clear the component data
    try self.components.remove(T, self._allocator, entity);

    // Unset the appropriate component bit for the entity
    const component_id = self.components.getComponentId(T) orelse return;
    self.entities.unsetSignatureBit(entity, @intCast(component_id));

    // Notify all systems to move the entity to the proper sets
    const signature = self.entities.getSignature(entity);
    try self.systems.entitySignatureChanged(self._allocator, entity, signature);
}

pub fn getComponent(self: Self, comptime T: type, entity: Entity) ?*T {
    return self.components.get(T, entity);
}

pub fn registerSystem(
    self: *Self,
    comptime components: []const type,
    schedule: Systems.Schedule,
    system: Systems.WorldSystem,
) !Systems.SystemId {
    const system_id = try self.systems.register(self._allocator, schedule, system);

    var signature: Signature = .initEmpty();
    inline for (components) |component| {
        const component_id = self.components.getComponentId(component) orelse {
            std.debug.panic("Component '{s}' has not been registered with the world.", .{
                @typeName(component),
            });
        };
        signature.set(@intCast(component_id));
    }
    try self.systems.setSignature(self._allocator, system_id, signature);

    return system_id;
}

pub fn unregisterSystem(self: *Self, system: Systems.SystemId) void {
    self.systems.unregister(self._allocator, system);
}

pub fn runSystems(self: *Self, schedule: Systems.Schedule) void {
    self.systems.run(schedule, self);
}

test "add entity" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    const entity = _world.createEntity();
    try std.testing.expectEqual(0, entity.id);
}

test "destroy entity" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    const entity1 = _world.createEntity();
    const entity2 = _world.createEntity();
    const entity3 = _world.createEntity();

    try std.testing.expectEqual(0, entity1.id);
    try std.testing.expectEqual(1, entity2.id);
    try std.testing.expectEqual(2, entity3.id);

    try _world.insertComponent(world.components.Transform, entity2, .{});

    try _world.destroyEntity(entity2);
    try std.testing.expectEqual(null, _world.getComponent(world.components.Transform, entity2));

    const entity4 = _world.createEntity();
    try std.testing.expectEqual(1, entity4.id);
}

test "add component" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    const entity = _world.createEntity();
    try _world.insertComponent(world.components.Transform, entity, .{
        .translation = .init(
            1.0,
            2.0,
            3.0,
            4.0,
        ),
    });

    const transform = _world.getComponent(world.components.Transform, entity) orelse unreachable;

    try std.testing.expectEqual(1.0, transform.translation.x());
    try std.testing.expectEqual(2.0, transform.translation.y());
    try std.testing.expectEqual(3.0, transform.translation.z());
    try std.testing.expectEqual(4.0, transform.translation.w());
}

test "remove component" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    const entity = _world.createEntity();
    try _world.insertComponent(world.components.Transform, entity, .{
        .translation = .splat(1.0),
    });

    const transform = _world.getComponent(world.components.Transform, entity) orelse unreachable;

    try std.testing.expectEqual(1.0, transform.translation.x());
    try std.testing.expectEqual(1.0, transform.translation.y());
    try std.testing.expectEqual(1.0, transform.translation.z());
    try std.testing.expectEqual(1.0, transform.translation.w());

    try _world.removeComponent(world.components.Transform, entity);
    try std.testing.expectEqual(null, _world.getComponent(world.components.Transform, entity));
}

test "update component" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    const entity = _world.createEntity();
    try _world.insertComponent(world.components.Transform, entity, .{
        .translation = .splat(1.0),
    });

    {
        const transform = _world.getComponent(world.components.Transform, entity) orelse unreachable;
        transform.translation = .init(4.0, 3.0, 2.0, 1.0);
    }

    {
        const transform = _world.getComponent(world.components.Transform, entity) orelse unreachable;

        try std.testing.expectEqual(4.0, transform.translation.x());
        try std.testing.expectEqual(3.0, transform.translation.y());
        try std.testing.expectEqual(2.0, transform.translation.z());
        try std.testing.expectEqual(1.0, transform.translation.w());
    }
}

const ComponentA = struct {
    count: usize = 0,
};

const ComponentB = struct {
    count: usize = 0,
};

fn systemA(param: SystemParam) void {
    var entities = param.entities.iterator();
    while (entities.next()) |entity| {
        const component = param.world.getComponent(ComponentA, entity.*) orelse continue;
        component.count += 1;
    }
}

fn systemAB(param: SystemParam) void {
    var entities = param.entities.iterator();
    while (entities.next()) |entity| {
        const a = param.world.getComponent(ComponentA, entity.*) orelse continue;
        const b = param.world.getComponent(ComponentB, entity.*) orelse continue;
        a.count += 2;
        b.count += 2;
    }
}

test "register system" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    try _world.registerComponent(ComponentA);
    _ = try _world.registerSystem(&.{ComponentA}, .startup, systemA);

    const entity = _world.createEntity();
    try _world.insertComponent(ComponentA, entity, .{});

    _world.runSystems(.startup);

    const component = _world.getComponent(ComponentA, entity) orelse unreachable;
    try std.testing.expectEqual(1, component.count);
}

test "unregister system" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    try _world.registerComponent(ComponentA);
    const system = try _world.registerSystem(&.{ComponentA}, .startup, systemA);

    const entity = _world.createEntity();
    try _world.insertComponent(ComponentA, entity, .{});

    _world.runSystems(.startup);

    const component = _world.getComponent(ComponentA, entity) orelse unreachable;
    try std.testing.expectEqual(1, component.count);

    _world.unregisterSystem(system);
    _world.runSystems(.startup);
    try std.testing.expectEqual(1, component.count);
}

test "multiple systems" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    try _world.registerComponents(&.{ComponentA, ComponentB});
    _ = try _world.registerSystem(&.{ComponentA}, .update, systemA);
    _ = try _world.registerSystem(&.{ComponentA, ComponentB}, .update, systemAB);

    const entityA = _world.createEntity();
    const entityAB = _world.createEntity();

    try _world.insertComponent(ComponentA, entityA, .{});
    try _world.insertComponent(ComponentA, entityAB, .{});
    try _world.insertComponent(ComponentB, entityAB, .{});

    _world.runSystems(.update);

    {
        const a = _world.getComponent(ComponentA, entityA) orelse unreachable;
        try std.testing.expectEqual(1, a.count);
    }

    {
        const a = _world.getComponent(ComponentA, entityAB) orelse unreachable;
        const b = _world.getComponent(ComponentB, entityAB) orelse unreachable;
        try std.testing.expectEqual(3, a.count);
        try std.testing.expectEqual(2, b.count);
    }
}

test "entity change systems" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    try _world.registerComponents(&.{ComponentA, ComponentB});
    _ = try _world.registerSystem(&.{ComponentA}, .update, systemA);
    _ = try _world.registerSystem(&.{ComponentA, ComponentB}, .update, systemAB);

    const entity = _world.createEntity();
    try _world.insertComponent(ComponentA, entity, .{});
    _world.runSystems(.update);

    {
        const a = _world.getComponent(ComponentA, entity) orelse unreachable;
        try std.testing.expectEqual(1, a.count);
    }

    try _world.insertComponent(ComponentB, entity, .{});
    _world.runSystems(.update);

    {
        const a = _world.getComponent(ComponentA, entity) orelse unreachable;
        const b = _world.getComponent(ComponentB, entity) orelse unreachable;
        try std.testing.expectEqual(4, a.count);
        try std.testing.expectEqual(2, b.count);
    }

    try _world.removeComponent(ComponentB, entity);
    _world.runSystems(.update);

    {
        const a = _world.getComponent(ComponentA, entity) orelse unreachable;
        try std.testing.expectEqual(5, a.count);
        try std.testing.expectEqual(null, _world.getComponent(ComponentB, entity));
    }

    try _world.destroyEntity(entity);
    _world.runSystems(.update);

    {
        try std.testing.expectEqual(null, _world.getComponent(ComponentA, entity));
    }
}
