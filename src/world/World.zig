const Components = @import("Components.zig");
const Entities = @import("Entities.zig");
const Resources = @import("Resources.zig");
const std = @import("std");
const world = @import("root.zig");

const Signature = Components.Signature;
const Camera = world.Camera;
const Entity = world.Entity;
const Systems = world.Systems;
const SystemParam = Systems.SystemParam;
const Transform = world.components.core.Transform;
const World = world.World;

const Self = @This();

pub const Error = error{
    DuplicateQuery,
};

entities: Entities,
components: Components,
systems: Systems,
resources: Resources,
_allocator: std.mem.Allocator,

pub fn init(allocator: std.mem.Allocator) !Self {
    var components: Components = .init();
    try components.register(Transform, allocator);

    var resources: Resources = .init();
    try resources.add(world.resources.core.Frame, allocator, .{});

    return .{
        .entities = .init(allocator),
        .components = components,
        .systems = .init(),
        .resources = resources,
        ._allocator = allocator,
    };
}

pub fn deinit(self: *Self) void {
    self.entities.deinit(self._allocator);
    self.components.deinit(self._allocator);
    self.systems.deinit(self._allocator);
    self.resources.deinit(self._allocator);
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

pub fn registerComponents(self: *Self, comptime components: []const type) !void {
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

pub fn hasComponent(self: Self, comptime T: type, entity: Entity) bool {
    const id = self.components.getComponentId(T) orelse return false;
    const signature = self.entities.getSignature(entity);
    return signature.isSet(id);
}

/// A system with 0 queries registered will always run. A system with registered queries
/// will only run if entities with matching components exist.
pub fn registerSystem(
    self: *Self,
    comptime queries: []const Systems.Query,
    schedule: Systems.Schedule,
    system: Systems.WorldSystem,
) !Systems.SystemId {
    const system_id = try self.systems.register(
        self._allocator,
        schedule,
        system,
        queries.len,
    );
    errdefer self.unregisterSystem(system_id);

    inline for (queries, 0..) |query, i| {
        var signature: Signature = .initEmpty();
        inline for (query.components) |component| {
            const component_id = self.components.getComponentId(component) orelse {
                std.debug.panic("Component '{s}' has not been registered with the world.", .{
                    @typeName(component),
                });
            };
            signature.set(@intCast(component_id));
        }

        if (self.systems.hasQuery(system_id, signature)) {
            return Error.DuplicateQuery;
        }

        try self.systems.setSignature(self._allocator, i, system_id, signature);
    }

    return system_id;
}

pub fn unregisterSystem(self: *Self, system: Systems.SystemId) void {
    self.systems.unregister(self._allocator, system);
}

pub fn runSystems(self: *Self, schedule: Systems.Schedule) void {
    self.systems.run(schedule, self);
}

pub fn registerResource(self: *Self, comptime T: type, resource: T) !void {
    try self.resources.add(T, self._allocator, resource);
}

pub fn getResource(self: Self, comptime T: type) ?*T {
    return self.resources.get(T);
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

    try _world.insertComponent(Transform, entity2, .{});

    try _world.destroyEntity(entity2);
    try std.testing.expectEqual(null, _world.getComponent(Transform, entity2));

    const entity4 = _world.createEntity();
    try std.testing.expectEqual(1, entity4.id);
}

test "add component" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    const entity = _world.createEntity();
    try _world.insertComponent(Transform, entity, .{
        .translation = .init(
            1.0,
            2.0,
            3.0,
            4.0,
        ),
    });

    const transform = _world.getComponent(Transform, entity) orelse unreachable;

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
    try _world.insertComponent(Transform, entity, .{
        .translation = .splat(1.0),
    });

    const transform = _world.getComponent(Transform, entity) orelse unreachable;

    try std.testing.expectEqual(1.0, transform.translation.x());
    try std.testing.expectEqual(1.0, transform.translation.y());
    try std.testing.expectEqual(1.0, transform.translation.z());
    try std.testing.expectEqual(1.0, transform.translation.w());

    try _world.removeComponent(Transform, entity);
    try std.testing.expectEqual(null, _world.getComponent(Transform, entity));
}

test "update component" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    const entity = _world.createEntity();
    try _world.insertComponent(Transform, entity, .{
        .translation = .splat(1.0),
    });

    {
        const transform = _world.getComponent(Transform, entity) orelse unreachable;
        transform.translation = .init(4.0, 3.0, 2.0, 1.0);
    }

    {
        const transform = _world.getComponent(Transform, entity) orelse unreachable;

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

const ComponentC = struct {
    count: usize = 0,
};

fn systemA(param: SystemParam) !void {
    var entities = param.queries[0].entities.iterator();
    while (entities.next()) |entity| {
        const component = param.world.getComponent(ComponentA, entity.*) orelse continue;
        component.count += 1;
    }
}

fn systemAB(param: SystemParam) !void {
    var entities = param.queries[0].entities.iterator();
    while (entities.next()) |entity| {
        const a = param.world.getComponent(ComponentA, entity.*) orelse continue;
        const b = param.world.getComponent(ComponentB, entity.*) orelse continue;
        a.count += 2;
        b.count += 2;
    }
}

fn systemABC(param: SystemParam) !void {
    var entities_ab = param.getEntities(0);
    while (entities_ab.next()) |entity| {
        const a = param.getComponent(ComponentA, entity.*) orelse continue;
        const b = param.getComponent(ComponentB, entity.*) orelse continue;
        a.count += 1;
        b.count += 2;
    }

    var entities_c = param.getEntities(1);
    while (entities_c.next()) |entity| {
        const c = param.getComponent(ComponentC, entity.*) orelse continue;
        c.count += 3;
    }
}

test "register system" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    try _world.registerComponent(ComponentA);
    _ = try _world.registerSystem(
        &.{.init(&.{ComponentA})},
        .startup,
        systemA,
    );

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
    const system = try _world.registerSystem(&.{.init(&.{ComponentA})}, .startup, systemA);

    const entity = _world.createEntity();
    try _world.insertComponent(ComponentA, entity, .{});

    _world.runSystems(.startup);

    const component = _world.getComponent(ComponentA, entity) orelse unreachable;
    try std.testing.expectEqual(1, component.count);

    _world.unregisterSystem(system);
    _world.runSystems(.startup);
    try std.testing.expectEqual(1, component.count);
}

test "multiple queries" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    try _world.registerComponents(&.{
        ComponentA,
        ComponentB,
        ComponentC,
    });

    _ = try _world.registerSystem(
        &.{
            .init(&.{ ComponentA, ComponentB }),
            .init(&.{ComponentC}),
        },
        .update,
        systemABC,
    );

    const entityAB = _world.createEntity();
    try _world.insertComponent(ComponentA, entityAB, .{});
    try _world.insertComponent(ComponentB, entityAB, .{});

    const entityC = _world.createEntity();
    try _world.insertComponent(ComponentC, entityC, .{});

    _world.runSystems(.update);

    const a = _world.getComponent(ComponentA, entityAB) orelse unreachable;
    const b = _world.getComponent(ComponentB, entityAB) orelse unreachable;
    const c = _world.getComponent(ComponentC, entityC) orelse unreachable;

    try std.testing.expectEqual(1, a.count);
    try std.testing.expectEqual(2, b.count);
    try std.testing.expectEqual(3, c.count);
}

test "multiple queries remove component" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    try _world.registerComponents(&.{
        ComponentA,
        ComponentB,
        ComponentC,
    });

    _ = try _world.registerSystem(
        &.{
            .init(&.{ ComponentA, ComponentB }),
            .init(&.{ComponentC}),
        },
        .update,
        systemABC,
    );

    const entityAB = _world.createEntity();
    try _world.insertComponent(ComponentA, entityAB, .{});
    try _world.insertComponent(ComponentB, entityAB, .{});

    const entityC = _world.createEntity();
    try _world.insertComponent(ComponentC, entityC, .{});

    _world.runSystems(.update);

    const a = _world.getComponent(ComponentA, entityAB) orelse unreachable;
    const b = _world.getComponent(ComponentB, entityAB) orelse unreachable;
    const c = _world.getComponent(ComponentC, entityC) orelse unreachable;

    try std.testing.expectEqual(1, a.count);
    try std.testing.expectEqual(2, b.count);
    try std.testing.expectEqual(3, c.count);

    try _world.removeComponent(ComponentA, entityAB);

    _world.runSystems(.update);

    try std.testing.expectEqual(1, a.count);
    try std.testing.expectEqual(2, b.count);
    try std.testing.expectEqual(6, c.count);
}

test "duplicate queries" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    try _world.registerComponents(&.{ ComponentA, ComponentB });

    const result = _world.registerSystem(
        &.{
            .init(&.{ ComponentA, ComponentB }),
            .init(&.{ ComponentB, ComponentA }),
        },
        .startup,
        systemAB,
    );
    try std.testing.expectEqual(Error.DuplicateQuery, result);
}

test "multiple systems" {
    const allocator = std.testing.allocator;

    var _world: Self = try .init(allocator);
    defer _world.deinit();

    try _world.registerComponents(&.{ ComponentA, ComponentB });
    _ = try _world.registerSystem(&.{.init(&.{ComponentA})}, .update, systemA);
    _ = try _world.registerSystem(&.{.init(&.{ ComponentA, ComponentB })}, .update, systemAB);

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

    try _world.registerComponents(&.{ ComponentA, ComponentB });
    _ = try _world.registerSystem(&.{.init(&.{ComponentA})}, .update, systemA);
    _ = try _world.registerSystem(&.{.init(&.{ ComponentA, ComponentB })}, .update, systemAB);

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

const TestResource = struct {
    value: u32 = 0,
};

fn systemResource(param: SystemParam) !void {
    const resource = param.world.getResource(TestResource) orelse unreachable;
    resource.value += 5;
}

test "resource" {
    const allocator = std.testing.allocator;

    var _world: World = try .init(allocator);
    defer _world.deinit();

    _ = try _world.registerSystem(&.{}, .update, systemResource);

    try _world.registerResource(TestResource, .{
        .value = 5,
    });

    {
        const resource = _world.getResource(TestResource) orelse unreachable;
        try std.testing.expectEqual(5, resource.value);
    }

    _world.runSystems(.update);

    {
        const resource = _world.getResource(TestResource) orelse unreachable;
        try std.testing.expectEqual(10, resource.value);
    }
}
