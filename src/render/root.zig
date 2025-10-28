pub const Camera = @import("Camera.zig");
pub const RenderBuffer = @import("RenderBuffer.zig");
pub const shaders = @import("shaders/root.zig");
pub const Vertex = @import("Vertex.zig");
pub const View = @import("View.zig");

const std = @import("std");

pub fn stateFlagsBlend(src: u64, dst: u64) u64 {
    return (src | (dst << 4)) | ((src | (dst << 4)) << 8);
}

test "refall" {
    std.testing.refAllDecls(@This());
}
