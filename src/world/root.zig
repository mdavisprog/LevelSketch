const std = @import("std");

pub const components = @import("components/root.zig");
pub const Entity = @import("Entity.zig");
pub const resources = @import("resources/root.zig");
pub const Systems = @import("Systems.zig");
pub const World = @import("World.zig");

test "refall" {
    std.testing.refAllDecls(@This());
}
