/// Represents a rotation using Euler angles.
const Self = @This();

pub const zero: Self = .{};

pitch: f32 = 0.0,
yaw: f32 = 0.0,
roll: f32 = 0.0,
