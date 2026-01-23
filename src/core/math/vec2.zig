const std = @import("std");
const Vec = @import("Vec.zig");

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

        pub fn to(self: Self, comptime U: type) Vec2(U) {
            if (T == U) {
                return self;
            }

            if (T == f32 or T == f64) {
                if (U != f32 and U != f64) {
                    return .init(@intFromFloat(self.x), @intFromFloat(self.y));
                } else {
                    return .init(@floatCast(self.x), @floatCast(self.y));
                }
            } else {
                if (U != f32 and U != f64) {
                    return .init(@intCast(self.x), @intCast(self.y));
                } else {
                    return .init(@floatFromInt(self.x), @floatFromInt(self.y));
                }
            }
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

        pub fn toVec(self: Self) Vec {
            if (T == f32) {
                return .init2(self.x, self.y);
            } else {
                return .init2(@floatFromInt(self.x), @floatFromInt(self.y));
            }
        }

        pub fn copy(self: *Self, other: Self) void {
            self.x = other.x;
            self.y = other.y;
        }

        pub fn isZero(self: Self) bool {
            return self.x == 0 and self.y == 0;
        }
    };
}
