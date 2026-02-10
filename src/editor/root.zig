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
pub const events = @import("events.zig");
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

        try world.registerEvent(events.ResetCamera);

        _ = try world.registerSystem(systems.updateCamera, .update);
        _ = try world.registerSystem(systems.orbit, .update);
        try world.registerEventListener(events.onResetCamera);

        const transform: _world.components.core.Transform = .{
            .translation = components.Camera.default_pos,
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

    pub fn addPointLight(self: *Self, renderer: *Renderer) !void {
        const cube = try render.shapes.cube(u16, renderer, .splat(0.2), 0xFFFFFFFF);
        _ = try self.world.createEntityWith(.{
            _world.components.core.Transform{},
            render.ecs.components.Mesh{
                .handle = cube,
            },
            render.ecs.components.Color{},
            components.Orbit{},
            render.ecs.components.Light{
                .ambient = .init(0.2, 0.2, 0.2, 1.0),
                .diffuse = .init(0.5, 0.5, 0.5, 1.0),
            },
            render.ecs.components.PointLight{},
        });
    }

    pub fn addDirectionalLight(self: *Self, direction: Vec) !void {
        _ = try self.world.createEntityWith(.{
            _world.components.core.Transform{
                .rotation = direction.toRotation(),
            },
            render.ecs.components.DirectionalLight{},
            render.ecs.components.Light{
                .ambient = .init(0.2, 0.2, 0.2, 1.0),
                .diffuse = .init(0.5, 0.5, 0.5, 1.0),
            },
        });
    }
};
