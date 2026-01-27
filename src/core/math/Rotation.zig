const math = @import("root.zig");
const std = @import("std");

const Vec = math.Vec;

/// Represents a euler rotation.
const Self = @This();

pub const zero: Self = .{};

pitch: f32 = 0.0,
yaw: f32 = 0.0,
roll: f32 = 0.0,

pub fn init(pitch: f32, yaw: f32, roll: f32) Self {
    return .{
        .pitch = pitch,
        .yaw = yaw,
        .roll = roll,
    };
}

pub fn pitchRad(self: Self) f32 {
    return std.math.degreesToRadians(self.pitch);
}

pub fn yawRad(self: Self) f32 {
    return std.math.degreesToRadians(self.yaw);
}

pub fn rollRad(self: Self) f32 {
    return std.math.degreesToRadians(self.roll);
}

pub fn toVec(self: Self) Vec {
    return .init(
        std.math.cos(self.yawRad()) * std.math.cos(self.pitchRad()),
        std.math.sin(self.pitchRad()),
        std.math.sin(self.yawRad()) * std.math.cos(self.pitchRad()),
        0.0,
    );
}

pub fn addPitch(self: *Self, delta: f32) void {
    self.pitch = @mod(self.pitch + delta, 360.0);
}

pub fn addYaw(self: *Self, delta: f32) void {
    self.yaw = @mod(self.yaw + delta, 360.0);
}

pub fn addRoll(self: *Self, delta: f32) void {
    self.roll = @mod(self.roll + delta, 360.0);
}
