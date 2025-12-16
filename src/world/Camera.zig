const core = @import("core");
const std = @import("std");

const Mat = core.math.Mat;
const Vec = core.math.Vec;

const Self = @This();

pub const Direction = enum {
    forward,
    backward,
    left,
    right,
    up,
    down,
};

position: Vec = .zero,
direction: Vec = Vec.forward,
velocity: Vec = .zero,
pitch: f32 = 0.0,
yaw: f32 = 0.0,
move_speed: f32 = 0.5,
max_speed: f32 = 40.0,
rotation_speed: f32 = 0.25,

pub fn toLookAt(self: Self) Mat {
    return .initLookToLh(self.position, self.direction, Vec.up);
}

pub fn move(self: *Self, where: Direction) void {
    const direction = switch (where) {
        .forward => self.direction,
        .backward => self.direction.mul(-1.0),
        .left => self.direction.cross(Vec.up),
        .right => self.direction.cross(Vec.up).mul(-1.0),
        .up => Vec.up,
        .down => Vec.up.mul(-1.0),
    };

    const delta = direction.mul(self.move_speed);
    self.velocity.addMut(delta);
}

pub fn update(self: *Self, delta_time: f32) void {
    self.velocity.clampMut(self.max_speed);
    self.position.addMut(self.velocity.mul(delta_time));

    const friction = 0.9;
    self.velocity.mulMut(friction);
}

pub fn rotate(self: *Self, pitch: f32, yaw: f32) void {
    self.pitch += pitch * self.rotation_speed;
    self.yaw += yaw * self.rotation_speed;

    const transform = Mat.initRotationX(self.pitch).mul(.initRotationY(self.yaw));
    self.direction = Vec.forward.mul(transform).normalize();
}

pub fn resetRotation(self: *Self) void {
    self.pitch = 0.0;
    self.yaw = 0.0;
    self.rotate(0.0, 0.0);
}
