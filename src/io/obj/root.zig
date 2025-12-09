pub const Model = @import("Model.zig");

const core = @import("core");
const io = @import("../root.zig");
const std = @import("std");

const Vec = core.math.Vec;

const LineReader = io.LineReader;

test "refall" {
    std.testing.refAllDecls(@This());
}
