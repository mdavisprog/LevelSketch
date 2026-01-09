const world = @import("root.zig");

const Camera = world.Camera;

const Self = @This();

camera: Camera = .{},
light_orbit: bool = true,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self) void {
    _ = self;
}
