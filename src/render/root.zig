pub const Camera = @import("Camera.zig");
pub const shaders = @import("shaders/root.zig");
pub const Vertex = @import("Vertex.zig");
pub const View = @import("View.zig");

const std = @import("std");

test "refall" {
    std.testing.refAllDecls(@This());
}
