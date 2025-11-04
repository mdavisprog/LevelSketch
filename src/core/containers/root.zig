const buffers = @import("buffers.zig");
const std = @import("std");

pub const Buffer2D = buffers.Buffer2D;

test "refall" {
    std.testing.refAllDecls(@This());
}
