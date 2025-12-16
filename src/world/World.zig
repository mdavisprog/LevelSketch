const world = @import("root.zig");

const Camera = world.Camera;

const Self = @This();

camera: Camera = .{},

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self) void {
    _ = self;
}
