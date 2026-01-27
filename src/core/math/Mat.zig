const math = @import("root.zig");
const std = @import("std");
const zmath = @import("zmath");

const Rotation = math.Rotation;
const Vec = math.Vec;

const Self = @This();

pub const identity: Self = .{};

data: zmath.Mat = zmath.identity(),

pub fn initTranslation(translation: Vec) Self {
    return .{
        .data = zmath.translation(translation.x(), translation.y(), translation.z()),
    };
}

pub fn initRotation(rotation: Rotation) Self {
    return .{
        .data = zmath.matFromRollPitchYaw(
            rotation.pitchRad(),
            rotation.yawRad(),
            rotation.rollRad(),
        ),
    };
}

pub fn initRotationX(angle_degrees: f32) Self {
    const angle = std.math.degreesToRadians(angle_degrees);
    return .{
        .data = zmath.rotationX(angle),
    };
}

pub fn initRotationY(angle_degrees: f32) Self {
    const angle = std.math.degreesToRadians(angle_degrees);
    return .{
        .data = zmath.rotationY(angle),
    };
}

pub fn initRotationZ(angle_degrees: f32) Self {
    const angle = std.math.degreesToRadians(angle_degrees);
    return .{
        .data = zmath.rotationZ(angle),
    };
}

pub fn initScale(_scale: Vec) Self {
    return .{
        .data = zmath.scaling(_scale.x(), _scale.y(), _scale.z()),
    };
}

pub fn initPerspectiveFovLh(fov_degrees: f32, aspect: f32) Self {
    const fov = std.math.degreesToRadians(fov_degrees);
    return .{
        .data = zmath.perspectiveFovLh(fov, aspect, 0.1, 100.0),
    };
}

pub fn initOrthographicOffCenterLh(width: f32, height: f32) Self {
    return .{
        .data = zmath.orthographicOffCenterLh(
            0.0,
            width,
            0.0,
            height,
            -1.0,
            1.0,
        ),
    };
}

pub fn initLookToLh(position: Vec, forward: Vec, up: Vec) Self {
    return .{
        .data = zmath.lookToLh(position.data, forward.data, up.data),
    };
}

pub fn mul(self: Self, other: Self) Self {
    return .{
        .data = zmath.mul(self.data, other.data),
    };
}

pub fn translate(self: Self, translation: Vec) Self {
    return self.mul(.initTranslation(translation));
}

pub fn rotate(self: Self, rotation: Rotation) Self {
    return self.mul(.initRotation(rotation));
}

pub fn rotateX(self: Self, degrees: f32) Self {
    return self.mul(.initRotationX(degrees));
}

pub fn rotateY(self: Self, degrees: f32) Self {
    return self.mul(.initRotationY(degrees));
}

pub fn rotateZ(self: Self, degrees: f32) Self {
    return self.mul(.initRotationZ(degrees));
}

pub fn scale(self: Self, _scale: Vec) Self {
    return self.mul(.initScale(_scale));
}

pub fn inverse(self: Self) Self {
    return .{
        .data = zmath.inverse(self.data),
    };
}

pub fn transpose(self: Self) Self {
    return .{
        .data = zmath.transpose(self.data),
    };
}

pub fn toArray(self: Self) [16]f32 {
    return zmath.matToArr(self.data);
}

pub fn getTranslation(self: Self) Vec {
    return .init(self.data[3][0], self.data[3][1], self.data[3][2], self.data[3][3]);
}
