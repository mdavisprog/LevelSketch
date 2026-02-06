const core = @import("core");
const render = @import("render");
const std = @import("std");
const systems = @import("systems.zig");
const _world = @import("world");

const Entity = _world.Entity;
const Mat = core.math.Mat;
const Renderer = render.Renderer;
const Vec = core.math.Vec;
const World = _world.World;

pub const components = @import("components.zig");
pub const resources = @import("resources.zig");

pub const Editor = struct {
    const Self = @This();

    world: *World,
    camera: Entity = .{},

    pub fn init(world: *World) !Self {
        try world.registerComponents(&.{
            components.Camera,
            components.Orbit,
        });

        try world.registerResource(resources.Orbit, .{});

        _ = try world.registerSystem(systems.updateCamera, .update);
        _ = try world.registerSystem(systems.orbit, .update);

        const transform: _world.components.core.Transform = .{
            .translation = .init(0.0, 0.0, -3.0, 1.0),
            .rotation = Vec.forward.toRotation(),
        };

        const camera = world.createEntity();
        try world.insertComponent(_world.components.core.Transform, camera, transform);
        try world.insertComponent(components.Camera, camera, .{});

        return .{
            .world = world,
            .camera = camera,
        };
    }

    pub fn deinit(self: *Self) void {
        _ = self;
    }

    pub fn addLight(self: *Self, renderer: *Renderer) !void {
        const cube = try render.shapes.cube(u16, renderer, .splat(0.2), 0xFFFFFFFF);
        const cube_entity = self.world.createEntity();
        try self.world.insertComponent(_world.components.core.Transform, cube_entity, .{});
        try self.world.insertComponent(render.ecs.components.Mesh, cube_entity, .{
            .handle = cube,
        });
        try self.world.insertComponent(render.ecs.components.Color, cube_entity, .{});
        try self.world.insertComponent(components.Orbit, cube_entity, .{});
        try self.world.insertComponent(render.ecs.components.Light, cube_entity, .{
            .ambient = .init(0.2, 0.2, 0.2, 1.0),
            .diffuse = .init(0.5, 0.5, 0.5, 1.0),
        });
    }
};
