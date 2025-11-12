const std = @import("std");
const zmath = @import("zmath");

const Self = @This();

pub const Direction = enum {
    forward,
    backward,
    left,
    right,
    up,
    down,
};

pub const up: zmath.Vec = zmath.f32x4(0.0, 1.0, 0.0, 0.0);
pub const forward: zmath.Vec = zmath.f32x4(0.0, 0.0, 1.0, 0.0);

position: zmath.Vec = zmath.f32x4(0.0, 0.0, 0.0, 1.0),
direction: zmath.Vec = forward,
velocity: zmath.Vec = zmath.f32x4s(0.0),
pitch: f32 = 0.0,
yaw: f32 = 0.0,
move_speed: f32 = 0.5,
max_speed: f32 = 40.0,
rotation_speed: f32 = 0.25,

pub fn toLookAt(self: Self) zmath.Mat {
    return zmath.lookToLh(self.position, self.direction, up);
}

pub fn move(self: *Self, where: Direction) void {
    const direction = switch (where) {
        .forward => self.direction,
        .backward => self.direction * zmath.f32x4s(-1.0),
        .left => zmath.cross3(self.direction, up),
        .right => zmath.cross3(self.direction, up) * zmath.f32x4s(-1.0),
        .up => up,
        .down => up * zmath.f32x4s(-1.0),
    };

    const delta = direction * zmath.f32x4s(self.move_speed);
    self.velocity += delta;
}

pub fn update(self: *Self, delta_time: f32) void {
    self.velocity = clamp(self.velocity, self.max_speed);
    self.position += self.velocity * zmath.f32x4s(delta_time);

    const friction = 0.9;
    self.velocity = self.velocity * zmath.f32x4s(friction);
}

pub fn rotate(self: *Self, pitch: f32, yaw: f32) void {
    self.pitch += pitch * self.rotation_speed;
    self.yaw += yaw * self.rotation_speed;

    const pitch_rad = std.math.degreesToRadians(self.pitch);
    const yaw_rad = std.math.degreesToRadians(self.yaw);

    const transform = zmath.mul(zmath.rotationX(pitch_rad), zmath.rotationY(yaw_rad));
    self.direction = zmath.normalize3(zmath.mul(forward, transform));
}

fn getDelta(self: Self, direction: zmath.Vec, delta_time: f32) zmath.Vec {
    return direction * zmath.f32x4s(self.move_speed) * zmath.f32x4s(delta_time);
}

fn clamp(vector: zmath.Vec, max: f32) zmath.Vec {
    var result = zmath.f32x4(vector[0], vector[1], vector[2], vector[3]);

    const current = zmath.lengthSq3(vector);
    if (current[0] <= 0.0 or max <= 0.0) {
        return result;
    }

    const length = zmath.sqrt(current);
    if (length[0] > max) {
        result = result * zmath.f32x4s(max / length[0]);
    }

    return result;
}
