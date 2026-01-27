const core = @import("core");
const render = @import("../root.zig");
const world = @import("world");

const Entity = world.Entity;
const HashSetUnmanaged = core.containers.HashSetUnmanaged;
const Renderer = render.Renderer;

pub const Render = struct {
    renderer: *Renderer,
};

pub const Lights = struct {
    entities: HashSetUnmanaged(world.Entity) = .empty,
};
