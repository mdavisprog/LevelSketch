const std = @import("std");

pub fn Vec3(comptime T: type) type {
    return struct {
        const Self = @This();

        pub const zero: Self = .{
            .x = 0,
            .y = 0,
            .z = 0,
        };

        x: T,
        y: T,
        z: T,

        pub fn init(x: T, y: T, z: T) Self {
            return Self{
                .x = x,
                .y = y,
                .z = z,
            };
        }

        pub fn splat(value: T) Self {
            return Self{
                .x = value,
                .y = value,
                .z = value,
            };
        }

        pub fn add(self: Self, other: Self) Self {
            return .{
                .x = self.x + other.x,
                .y = self.y + other.y,
                .z = self.z + other.z,
            };
        }

        pub fn addMut(self: *Self, other: Self) *Self {
            self.x += other.x;
            self.y += other.y;
            self.z += other.z;
            return self;
        }

        pub fn sub(self: Self, other: Self) Self {
            return .{
                .x = self.x - other.x,
                .y = self.y - other.y,
                .z = self.z - other.z,
            };
        }

        pub fn subMut(self: *Self, other: Self) *Self {
            self.x -= other.x;
            self.y -= other.y;
            self.z -= other.z;
            return self;
        }

        pub fn mul(self: Self, other: Self) Self {
            return .{
                .x = self.x * other.x,
                .y = self.y * other.y,
                .z = self.z * other.z,
            };
        }

        pub fn mulScalar(self: Self, scalar: T) Self {
            return .{
                .x = self.x * scalar,
                .y = self.y * scalar,
                .z = self.z * scalar,
            };
        }

        pub fn mulMut(self: *Self, other: Self) *Self {
            self.x *= other.x;
            self.y *= other.y;
            self.z *= other.z;
            return self;
        }

        pub fn div(self: Self, other: Self) Self {
            return .{
                .x = self.x / other.x,
                .y = self.y / other.y,
                .z = self.z / other.z,
            };
        }

        pub fn divMut(self: *Self, other: Self) *Self {
            self.x /= other.x;
            self.y /= other.y;
            self.z /= other.z;
            return self;
        }

        pub fn length(self: Self) T {
            return std.math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z);
        }

        pub fn lengthSq(self: Self) T {
            return self.x * self.x + self.y * self.y + self.z * self.z;
        }

        pub fn normalized(self: Self) Self {
            const len = self.length();
            if (len == 0.0) {
                return .zero;
            }

            return self.div(len);
        }
    };
}
