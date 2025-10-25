const std = @import("std");
const zmath = @import("zmath");

const Self = @This();

pub const UP: zmath.Vec = zmath.f32x4(0.0, 1.0, 0.0, 0.0);
pub const FORWARD: zmath.Vec = zmath.f32x4(0.0, 0.0, 1.0, 0.0);

position: zmath.Vec = zmath.f32x4(0.0, 0.0, 0.0, 1.0),
direction: zmath.Vec = FORWARD,
pitch: f32 = 0.0,
yaw: f32 = 0.0,
move_speed: f32 = 10.0,
rotation_speed: f32 = 0.25,

pub fn toLookAt(self: Self) zmath.Mat {
    return zmath.lookToLh(self.position, self.direction, UP);
}

pub fn moveForward(self: *Self, delta_time: f32) void {
    const delta = self.getDelta(self.direction, delta_time);
    self.position += delta;
}

pub fn moveBackward(self: *Self, delta_time: f32) void {
    const delta = self.getDelta(self.direction, delta_time);
    self.position -= delta;
}

pub fn moveRight(self: *Self, delta_time: f32) void {
    const right = zmath.cross3(self.direction, UP);
    const delta = self.getDelta(right, delta_time);
    self.position -= delta;
}

pub fn moveLeft(self: *Self, delta_time: f32) void {
    const right = zmath.cross3(self.direction, UP);
    const delta = self.getDelta(right, delta_time);
    self.position += delta;
}

pub fn rotate(self: *Self, pitch: f32, yaw: f32) void {
    self.pitch += pitch * self.rotation_speed;
    self.yaw += yaw * self.rotation_speed;

    const pitch_rad = std.math.degreesToRadians(self.pitch);
    const yaw_rad = std.math.degreesToRadians(self.yaw);

    const transform = zmath.mul(zmath.rotationX(pitch_rad), zmath.rotationY(yaw_rad));
    self.direction = zmath.normalize3(zmath.mul(FORWARD, transform));
}

fn getDelta(self: Self, direction: zmath.Vec, delta_time: f32) zmath.Vec {
    return direction * zmath.f32x4s(self.move_speed) * zmath.f32x4s(delta_time);
}
