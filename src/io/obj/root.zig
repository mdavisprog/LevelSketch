const core = @import("core");
const std = @import("std");

const Vec = core.math.Vec;

pub const Material = @import("Material.zig");
pub const Model = @import("Model.zig");

test "refall" {
    std.testing.refAllDecls(@This());
}
