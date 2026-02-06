const core = @import("core");
const std = @import("std");

pub const components = @import("components/root.zig");
const Entities = @import("Entities.zig");
const Queries = @import("Queries.zig");
pub const resources = @import("resources/root.zig");
pub const Systems = @import("Systems.zig");
pub const World = @import("World.zig");

pub const Entity = Entities.Entity;
pub const Handle = core.Handle;
pub const HashSetUnmanaged = core.containers.HashSetUnmanaged;
pub const Query = Queries.Query;

test "refall" {
    std.testing.refAllDecls(@This());
}
