const world = @import("world");

const World = world.World;

pub const Keyboard = @import("Keyboard.zig");
pub const Mouse = @import("Mouse.zig");
pub const resources = @import("resources.zig");

pub const Action = enum {
    press,
    release,
    repeat,
};
