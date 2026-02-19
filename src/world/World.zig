const Components = @import("Components.zig");
const Entities = @import("Entities.zig");
const Events = @import("Events.zig");
const Queries = @import("Queries.zig");
const Resources = @import("Resources.zig");
const std = @import("std");
const world = @import("root.zig");

const Signature = Components.Signature;
const Camera = world.Camera;
const Entity = world.Entity;
const Query = Queries.Query;
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
queries: *Queries,
events: *Events,
_allocator: std.mem.Allocator,

/// Creates a world object on the heap. This is due to needing to register systems that require
/// a pointer to the world when a SystemParam parameter is found.
pub fn init(allocator: std.mem.Allocator) !*Self {
    const queries = try allocator.create(Queries);
    errdefer allocator.destroy(queries);
    queries.* = .init();

    const events = try allocator.create(Events);
    errdefer allocator.destroy(events);
    events.* = .init();

    const result = try allocator.create(Self);
    result.* = .{
        .entities = .init(allocator),
        .components = .init(),
        .systems = .init(),
        .resources = .init(),
        .queries = queries,
        .events = events,
        ._allocator = allocator,
    };
    errdefer allocator.destroy(result);

    try result.registerComponents(&.{
        world.components.core.Transform,
    });

    return result;
}

pub fn deinit(self: *Self) void {
    self.entities.deinit(self._allocator);
    self.components.deinit(self._allocator);
    self.systems.deinit(self._allocator);
    self.resources.deinit(self._allocator);
    self.queries.deinit(self._allocator);
    self.events.deinit(self._allocator);

    self._allocator.destroy(self.queries);
    self._allocator.destroy(self.events);
}

pub fn createEntity(self: *Self) Entity {
    return self.entities.create();
}

pub fn createEntityWith(self: *Self, components: anytype) !Entity {
    const entity = self.createEntity();
    try self.insertComponents(entity, components);
    return entity;
}

pub fn duplicateEntity(self: *Self, entity: Entity) !Entity {
    const new_entity = self.createEntity();

    const signature = self.entities.getSignature(entity);
    self.entities.setSignature(new_entity, signature);
    try self.queries.signatureChanged(self._allocator, signature, new_entity);

    var it = signature.iterator(.{});
    while (it.next()) |component| {
        const data = self.components.getById(@intCast(component), entity) orelse continue;
        try self.components.insertById(self._allocator, @intCast(component), new_entity, data);
    }

    return new_entity;
}

pub fn destroyEntity(self: *Self, entity: Entity) !void {
    try self.entities.destroy(self._allocator, entity);
    try self.components.entityDestroyed(self._allocator, entity);
    self.queries.entityDestroyed(entity);
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
    try self.queries.signatureChanged(self._allocator, signature, entity);
}

pub fn insertComponents(self: *Self, entity: Entity, components: anytype) !void {
    const ComponentsType = @TypeOf(components);
    const components_info = @typeInfo(ComponentsType);
    if (components_info != .@"struct") {
        @compileError(std.fmt.comptimePrint(
            "Given components must be a struct or tuple. Type is {}.",
            .{ComponentsType},
        ));
    }

    inline for (std.meta.fields(ComponentsType)) |field| {
        try self.components.insert(
            field.type,
            self._allocator,
            entity,
            @field(components, field.name),
        );

        const id = self.components.getComponentId(field.type) orelse {
            std.debug.panic(
                "Failed to insert component '{s}'. Component not registered.",
                .{@typeName(field.type)},
            );
        };
        self.entities.setSignatureBit(entity, @intCast(id));
    }

    const signature = self.entities.getSignature(entity);
    try self.queries.signatureChanged(self._allocator, signature, entity);
}

pub fn removeComponent(self: *Self, comptime T: type, entity: Entity) !void {
    // Clear the component data
    try self.components.remove(T, self._allocator, entity);

    // Unset the appropriate component bit for the entity
    const component_id = self.components.getComponentId(T) orelse return;
    self.entities.unsetSignatureBit(entity, @intCast(component_id));

    // Notify all systems to move the entity to the proper sets
    const signature = self.entities.getSignature(entity);
    try self.queries.signatureChanged(self._allocator, signature, entity);
}

pub fn getComponent(self: Self, comptime T: type, entity: Entity) ?*T {
    return self.components.get(T, entity);
}

pub fn hasComponent(self: Self, comptime T: type, entity: Entity) bool {
    const id = self.components.getComponentId(T) orelse return false;
    const signature = self.entities.getSignature(entity);
    return signature.isSet(id);
}

/// Registers the given system with the world. This function will reflect on the system and create
/// queries based on the parameter types given to the system. A Query object contains a pointer
/// to the entity set so that systems can iterate over valid entities.
///
/// The system must contain at least 1 parameter of type Query. The system can also contain only one
/// SystemParam type.
pub fn registerSystem(
    self: *Self,
    system: anytype,
    schedule: Systems.Schedule,
) !Systems.SystemId {
    verifySystem(system);

    // Get the tuple type and instance it. This instance will be passed in as parameters to the
    // given system.
    const params = try self.parseSystemParams(system);

    const system_id = try self.systems.register(self._allocator, system, params, schedule);
    return system_id;
}

pub fn unregisterSystem(self: *Self, system: Systems.SystemId) void {
    self.systems.unregister(self._allocator, system);
}

pub fn runSystems(self: *Self, schedule: Systems.Schedule) void {
    self.systems.run(schedule);
}

pub fn registerResource(self: *Self, comptime T: type, resource: T) !void {
    try self.resources.add(T, self._allocator, resource);
}

pub fn getResource(self: Self, comptime T: type) ?*T {
    return self.resources.get(T);
}

pub fn registerEvent(self: *Self, comptime Event: type) !void {
    try self.events.add(self._allocator, Event);
}

pub fn registerEvents(self: *Self, comptime EventTypes: []const type) !void {
    inline for (EventTypes) |Event| {
        try self.events.add(self._allocator, Event);
    }
}

/// Same as registerSystem, but the system is not added to a schedule.
pub fn registerEventListener(self: *Self, system: anytype) !void {
    verifySystem(system);
    const params = try self.parseSystemParams(system);
    const event_type = blk: {
        const params_info = @typeInfo(@TypeOf(params.*));
        break :blk params_info.@"struct".fields[0].type;
    };
    const system_id = try self.systems.registerStandalone(self._allocator, system, params);
    try self.events.addListener(event_type, self._allocator, system_id);
}

pub fn triggerEvent(self: Self, event: anytype) void {
    const event_type = @TypeOf(event);
    const listeners = self.events.getListeners(event_type);
    for (listeners) |system| {
        _ = self.systems.setFirstSystemParam(system, event);
        self.systems.runId(system);
    }
}

fn parseSystemParams(self: *Self, system: anytype) !*std.meta.ArgsTuple(@TypeOf(system)) {
    // Get the tuple type and instance it. This instance will be passed in as parameters to the
    // given system.
    const ParametersType = std.meta.ArgsTuple(@TypeOf(system));
    const params = try self._allocator.create(ParametersType);
    errdefer self._allocator.destroy(params);

    comptime var found_system_param = false;
    const system_fn = @typeInfo(@TypeOf(system)).@"fn";
    var queries: [system_fn.params.len]Signature = @splat(.initEmpty());
    // Loop through each parameter and update the tuple instance based on the parameter type.
    inline for (system_fn.params, 0..) |param, i| {
        const param_type = param.type orelse {
            @compileError(std.fmt.comptimePrint(
                "Argument at index {} does not have a type",
                .{i},
            ));
        };

        const field = std.fmt.comptimePrint("{}", .{i});
        if (Queries.isQueryType(param_type)) {
            // This is a query object. Instance a default version of this to extract what components
            // are a part of this query. The runtime will create a signature based on registered
            // components and create a query to hold the entity set.
            var signature: Signature = .initEmpty();

            const param_default = std.mem.zeroInit(param_type, .{});
            inline for (param_default.components) |component| {
                const component_info = @typeInfo(component);
                if (component_info != .@"struct") {
                    @compileError(std.fmt.comptimePrint(
                        "Query component '{}' is not a struct in system {}.",
                        .{ component, @TypeOf(system) },
                    ));
                }

                const id = self.components.getComponentId(component) orelse {
                    std.debug.panic(
                        "Failed to generate query. Component '{s}' is not registered.",
                        .{@typeName(component)},
                    );
                };
                signature.set(@intCast(id));
            }

            // Check for duplicate queries within the system.
            for (queries) |query| {
                if (query.eql(signature)) {
                    return Error.DuplicateQuery;
                }
            }
            queries[i] = signature;

            // Update the tuple instance with the query object pointing to the entity set.
            const entities = try self.queries.add(param_type, self._allocator, signature);
            @field(params, field) = .{
                .entities = entities,
            };
        } else if (param_type == SystemParam) {
            if (found_system_param) {
                @compileError(std.fmt.comptimePrint(
                    "Multiple 'SystemParam' parameters found in system {}. Only one can " ++
                        "exist at a time.",
                    .{@TypeOf(system)},
                ));
            }
            found_system_param = true;

            // Set up the SystemParam argument to be used with the system.
            @field(params, field) = .{
                .world = self,
                .allocator = self._allocator,
            };
        } else {
            // If this parameter is a struct, check to see if it is a registered event. Events
            // must be the first parameter in the system and registered.
            if (self.events.contains(param_type)) {
                if (i != 0) {
                    std.debug.panic(
                        "Event system '{s}' does not have the event as the first parameter. " ++
                            "Event systems must have the event type as the first parameter.",
                        .{@typeName(@TypeOf(system))},
                    );
                }

                @field(params, field) = .{};
            } else {
                std.debug.panic(
                    "Invalid parameter type '{s}' for system '{s}'.",
                    .{ @typeName(param_type), @typeName(@TypeOf(system)) },
                );
            }
        }
    }

    return params;
}

fn verifySystem(system: anytype) void {
    const system_info = @typeInfo(@TypeOf(system));
    if (system_info != .@"fn") {
        @compileError(std.fmt.comptimePrint(
            "Given system {} is not a function.",
            .{@TypeOf(system)},
        ));
    }

    const system_fn = system_info.@"fn";
    if (system_fn.params.len == 0) {
        @compileError("Given system has no parameters. The system must have either a Query, " ++
            "a SystemParam, or an Event.");
    }
}

fn destroyWorld(_world: *World) void {
    _world.deinit();
    _world._allocator.destroy(_world);
}

test "add entity" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    const entity = _world.createEntity();
    try std.testing.expectEqual(0, entity.id);
}

test "destroy entity" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

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

test "duplicate entity" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponent(ComponentA);

    const entity = try _world.createEntityWith(.{ComponentA{ .count = 1 }});
    const duplicate = try _world.duplicateEntity(entity);

    try std.testing.expectEqual(0, entity.id);
    try std.testing.expectEqual(1, duplicate.id);

    const entity_a = _world.getComponent(ComponentA, entity) orelse unreachable;
    const duplicate_a = _world.getComponent(ComponentA, duplicate) orelse unreachable;

    try std.testing.expectEqual(entity_a.count, duplicate_a.count);
}

test "invalid entity" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try std.testing.expectError(
        Components.Error.InvalidEntity,
        _world.insertComponent(Transform, .invalid, .{}),
    );
}

test "add component" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

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

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

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

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

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

fn systemA(a: Query(&.{ComponentA}), param: SystemParam) !void {
    var entities = a.getEntities();
    while (entities.next()) |entity| {
        const component = param.world.getComponent(ComponentA, entity.*) orelse continue;
        component.count += 1;
    }
}

fn systemAB(ab: Query(&.{ ComponentA, ComponentB }), param: SystemParam) !void {
    var entities = ab.getEntities();
    while (entities.next()) |entity| {
        const a = param.world.getComponent(ComponentA, entity.*) orelse continue;
        const b = param.world.getComponent(ComponentB, entity.*) orelse continue;
        a.count += 2;
        b.count += 2;
    }
}

fn systemABC(
    ab: Query(&.{ ComponentA, ComponentB }),
    c: Query(&.{ComponentC}),
    param: SystemParam,
) !void {
    var entities_ab = ab.getEntities();
    while (entities_ab.next()) |entity| {
        if (param.getComponent(ComponentA, entity.*)) |a| {
            a.count += 1;
        }

        if (param.getComponent(ComponentB, entity.*)) |b| {
            b.count += 2;
        }
    }

    var entities_c = c.getEntities();
    while (entities_c.next()) |entity| {
        const _c = param.getComponent(ComponentC, entity.*) orelse continue;
        _c.count += 3;
    }
}

test "register system" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponent(ComponentA);
    _ = try _world.registerSystem(systemA, .startup);

    const entity = _world.createEntity();
    try _world.insertComponent(ComponentA, entity, .{});

    _world.runSystems(.startup);

    const component = _world.getComponent(ComponentA, entity) orelse unreachable;
    try std.testing.expectEqual(1, component.count);
}

test "unregister system" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponent(ComponentA);
    const system = try _world.registerSystem(systemA, .startup);

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

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponents(&.{
        ComponentA,
        ComponentB,
        ComponentC,
    });

    _ = try _world.registerSystem(systemABC, .update);

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

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponents(&.{
        ComponentA,
        ComponentB,
        ComponentC,
    });

    _ = try _world.registerSystem(systemABC, .update);

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

fn systemDuplicate(
    _: Query(&.{ ComponentA, ComponentB }),
    _: Query(&.{ ComponentB, ComponentA }),
) !void {}

test "duplicate queries" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponents(&.{ ComponentA, ComponentB });

    const result = _world.registerSystem(systemDuplicate, .startup);
    try std.testing.expectEqual(Error.DuplicateQuery, result);
}

test "multiple systems" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponents(&.{ ComponentA, ComponentB });
    _ = try _world.registerSystem(systemA, .update);
    _ = try _world.registerSystem(systemAB, .update);

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

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponents(&.{ ComponentA, ComponentB });
    _ = try _world.registerSystem(systemA, .update);
    _ = try _world.registerSystem(systemAB, .update);

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

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    _ = try _world.registerSystem(systemResource, .update);

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

test "insert components" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponents(&.{ ComponentA, ComponentB });

    const entity = _world.createEntity();
    try _world.insertComponents(
        entity,
        .{
            ComponentA{ .count = 1 },
            ComponentB{ .count = 2 },
        },
    );

    const a = _world.getComponent(ComponentA, entity) orelse unreachable;
    const b = _world.getComponent(ComponentB, entity) orelse unreachable;

    try std.testing.expectEqual(1, a.count);
    try std.testing.expectEqual(2, b.count);
}

const EventA = struct {
    count: u32 = 0,
};

fn systemEventA(event: EventA, query: Query(&.{ComponentA}), param: SystemParam) !void {
    var entities = query.getEntities();
    while (entities.next()) |entity| {
        const a = param.getComponent(ComponentA, entity.*) orelse continue;
        a.count = event.count;
    }
}

test "event" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponents(&.{ComponentA});
    try _world.registerEvent(EventA);
    try _world.registerEventListener(systemEventA);

    const entity = try _world.createEntityWith(.{
        ComponentA{},
    });

    const event: EventA = .{ .count = 5 };
    _world.triggerEvent(event);

    const a = _world.getComponent(ComponentA, entity) orelse unreachable;
    try std.testing.expectEqual(event.count, a.count);
}

const MarkerComponent = struct {};

fn systemMarker(markers: Query(&.{MarkerComponent}), param: SystemParam) !void {
    try std.testing.expectEqual(1, markers.numEntities());

    var entities = markers.getEntities();
    while (entities.next()) |entity| {
        try std.testing.expectEqual(null, param.getComponent(MarkerComponent, entity.*));
    }
}

test "marker component" {
    const allocator = std.testing.allocator;

    const _world: *Self = try .init(allocator);
    defer destroyWorld(_world);

    try _world.registerComponent(MarkerComponent);
    _ = try _world.registerSystem(systemMarker, .startup);
    _ = try _world.createEntityWith(.{MarkerComponent{}});

    _world.runSystems(.startup);
}
