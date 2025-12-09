const math = @import("root.zig");
const std = @import("std");
const vec2 = @import("vec2.zig");
const zmath = @import("zmath");

const Mat = math.Mat;
const Vec2f = vec2.Vec2(f32);

/// A 4 component vector. All 3D/4D operations should use this struct at it wraps around zmath.
const Self = @This();

pub const zero: Self = .splat(0.0);
pub const right: Self = .init(1.0, 0.0, 0.0, 0.0);
pub const up: Self = .init(0.0, 1.0, 0.0, 0.0);
pub const forward: Self = .init(0.0, 0.0, 1.0, 0.0);

data: zmath.Vec = zmath.f32x4s(0.0),

pub fn init(_x: f32, _y: f32, _z: f32, _w: f32) Self {
    return .{
        .data = zmath.f32x4(_x, _y, _z, _w),
    };
}

pub fn init3(_x: f32, _y: f32, _z: f32) Self {
    return .init(_x, _y, _z, 0.0);
}

pub fn init2(_x: f32, _y: f32) Self {
    return .init(_x, _y, 0.0, 0.0);
}

pub fn splat(value: f32) Self {
    return .{
        .data = zmath.f32x4s(value),
    };
}

pub fn x(self: Self) f32 {
    return self.data[0];
}

pub fn y(self: Self) f32 {
    return self.data[1];
}

pub fn z(self: Self) f32 {
    return self.data[2];
}

pub fn w(self: Self) f32 {
    return self.data[3];
}

pub fn setX(self: *Self, _x: f32) void {
    self.data[0] = _x;
}

pub fn setY(self: *Self, _y: f32) void {
    self.data[1] = _y;
}

pub fn setZ(self: *Self, _z: f32) void {
    self.data[2] = _z;
}

pub fn setW(self: *Self, _w: f32) void {
    self.data[3] = _w;
}

pub fn eql(self: Self, other: Self) bool {
    const result = zmath.isNearEqual(self.data, other.data, zmath.f32x4s(std.math.floatEps(f32)));
    return result[0] == true and
        result[1] == true and
        result[2] == true and
        result[3] == true;
}

pub fn add(self: Self, other: anytype) Self {
    return self.operation(other, .add);
}

pub fn addMut(self: *Self, other: anytype) void {
    self.data = self.add(other).data;
}

pub fn sub(self: Self, other: anytype) Self {
    return self.operation(other, .sub);
}

pub fn mul(self: Self, other: anytype) Self {
    return self.operation(other, .mul);
}

pub fn mulMut(self: *Self, other: anytype) void {
    self.data = self.mul(other).data;
}

pub fn normalize(self: Self) Self {
    return .{
        .data = zmath.normalize3(self.data),
    };
}

pub fn cross(self: Self, other: Self) Self {
    return .{
        .data = zmath.cross3(self.data, other.data),
    };
}

pub fn clamp(self: Self, max: f32) Self {
    var data = dupe(self.data);

    const current = zmath.lengthSq3(data);
    if (current[0] <= 0.0 or max <= 0.0) {
        return .{
            .data = data,
        };
    }

    const length = zmath.sqrt(current);
    if (length[0] > max) {
        data = data * zmath.f32x4s(max / length[0]);
    }

    return .{
        .data = data,
    };
}

pub fn clampMut(self: *Self, max: f32) void {
    self.data = self.clamp(max).data;
}

pub fn toArray(self: Self) [4]f32 {
    return zmath.vecToArr4(self.data);
}

pub fn toVec2(self: Self) Vec2f {
    return .init(self.data[0], self.data[1]);
}

fn operation(self: Self, other: anytype, op: Operation) Self {
    const OtherType = @TypeOf(other);

    if (OtherType == f32 or OtherType == f64 or OtherType == comptime_float) {
        const val: f32 = if (OtherType == f64) @floatCast(other) else other;
        const rhs = zmath.f32x4s(val);

        return switch (op) {
            .add => .{
                .data = self.data + rhs,
            },
            .sub => .{
                .data = self.data - rhs,
            },
            .mul => .{
                .data = self.data * rhs,
            },
            .div => .{
                .data = self.data / rhs,
            },
        };
    } else if (OtherType == Self) {
        return switch (op) {
            .add => .{
                .data = self.data + other.data,
            },
            .sub => .{
                .data = self.data - other.data,
            },
            .mul => .{
                .data = self.data * other.data,
            },
            .div => .{
                .data = self.data / other.data,
            },
        };
    } else if (OtherType == Mat) {
        return switch (op) {
            .add => {
                std.debug.panic("Cannot add Vec to Mat", .{});
            },
            .sub => {
                std.debug.panic("Cannot sub Vec from Mat", .{});
            },
            .mul => .{
                .data = zmath.mul(self.data, other.data),
            },
            .div => {
                std.debug.panic("Cannot div Vec from Mat", .{});
            },
        };
    } else {
        @compileError(std.fmt.comptimePrint(
            "Vec.operation => Invalid type: {s}",
            .{@typeName(OtherType)},
        ));
    }
}

fn dupe(value: zmath.Vec) zmath.Vec {
    return zmath.f32x4(
        value[0],
        value[1],
        value[2],
        value[3],
    );
}

const Operation = enum {
    add,
    sub,
    mul,
    div,
};
