const core = @import("core");

const Vec = core.math.Vec;

pub const Camera = struct {
    velocity: Vec = .zero,
    move_speed: f32 = 0.5,
    max_speed: f32 = 40.0,
    rotation_speed: f32 = 0.25,
    is_rotating: bool = false,
};

pub const Orbit = struct {
    yaw: f32 = 0.0,
    elapsed: f32 = 0.0,
    speed: f32 = 20.0,
};
