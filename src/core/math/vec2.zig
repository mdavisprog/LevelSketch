const std = @import("std");
const vec3 = @import("vec3.zig");

pub fn Vec2(comptime T: type) type {
    return struct {
        const Self = @This();

        pub const zero: Self = .{ .x = 0, .y = 0 };

        x: T = 0,
        y: T = 0,

        pub fn init(x: T, y: T) Self {
            return Self{
                .x = x,
                .y = y,
            };
        }

        pub fn splat(value: T) Self {
            return Self{
                .x = value,
                .y = value,
            };
        }

        pub fn add(self: Self, other: Self) Self {
            return Self{
                .x = self.x + other.x,
                .y = self.y + other.y,
            };
        }

        pub fn addMut(self: *Self, other: Self) *Self {
            self.x += other.x;
            self.y += other.y;
            return self;
        }

        pub fn sub(self: Self, other: Self) Self {
            return Self{
                .x = self.x - other.x,
                .y = self.y - other.y,
            };
        }

        pub fn subMut(self: *Self, other: Self) *Self {
            self.x -= other.x;
            self.y -= other.y;
            return self;
        }

        pub fn mul(self: Self, other: Self) Self {
            return Self{
                .x = self.x * other.x,
                .y = self.y * other.y,
            };
        }

        pub fn mulScalar(self: Self, scalar: T) Self {
            return Self{
                .x = self.x * scalar,
                .y = self.y * scalar,
            };
        }

        pub fn mulMut(self: *Self, other: Self) *Self {
            self.x *= other.x;
            self.y *= other.y;
            return self;
        }

        pub fn div(self: Self, other: Self) Self {
            return Self{
                .x = self.x / other.x,
                .y = self.y / other.y,
            };
        }

        pub fn divMut(self: *Self, other: Self) *Self {
            self.x /= other.x;
            self.y /= other.y;
            return self;
        }

        pub fn length(self: Self) T {
            return std.math.sqrt(self.x * self.x + self.y * self.y);
        }

        pub fn lengthSq(self: Self) T {
            return self.x * self.x + self.y * self.y;
        }

        pub fn normalized(self: Self) Self {
            const len = self.length();
            if (len == 0.0) {
                return .zero;
            }

            return self.div(len);
        }

        pub fn xyz(self: Self, z: T) vec3.Vec3(T) {
            return .init(self.x, self.y, z);
        }
    };
}
