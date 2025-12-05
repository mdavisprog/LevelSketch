const vec3 = @import("vec3.zig");

const Vec3 = vec3.Vec3;

pub fn Vec4(comptime T: type) type {
    return struct {
        const Self = @This();

        pub const zero: Self = .{
            .x = 0,
            .y = 0,
            .z = 0,
            .w = 0,
        };

        x: T,
        y: T,
        z: T,
        w: T,

        pub fn init(x: T, y: T, z: T, w: T) Self {
            return Self{
                .x = x,
                .y = y,
                .z = z,
                .w = w,
            };
        }

        pub fn splat(value: T) Self {
            return Self{
                .x = value,
                .y = value,
                .z = value,
                .w = value,
            };
        }

        pub fn eql(self: Self, other: Self) bool {
            return self.x == other.x and
                self.y == other.y and
                self.z == other.z and
                self.w == other.w;
        }

        pub fn toArray(self: Self) [4]T {
            return .{ self.x, self.y, self.z, self.w };
        }

        pub fn add(self: Self, other: Self) Self {
            return .{
                .x = self.x + other.x,
                .y = self.y + other.y,
                .z = self.z + other.z,
                .w = self.w + other.w,
            };
        }

        pub fn addMut(self: *Self, other: Self) *Self {
            self.x += other.x;
            self.y += other.y;
            self.z += other.z;
            self.w += other.w;
            return self;
        }

        pub fn sub(self: Self, other: Self) Self {
            return .{
                .x = self.x - other.x,
                .y = self.y - other.y,
                .z = self.z - other.z,
                .w = self.w - other.w,
            };
        }

        pub fn subMut(self: *Self, other: Self) *Self {
            self.x -= other.x;
            self.y -= other.y;
            self.z -= other.z;
            self.w -= other.w;
            return self;
        }

        pub fn mul(self: Self, other: Self) Self {
            return .{
                .x = self.x * other.x,
                .y = self.y * other.y,
                .z = self.z * other.z,
                .w = self.w * other.w,
            };
        }

        pub fn mulScalar(self: Self, scalar: T) Self {
            return .{
                .x = self.x * scalar,
                .y = self.y * scalar,
                .z = self.z * scalar,
                .w = self.w * scalar,
            };
        }

        pub fn mulMut(self: *Self, other: Self) *Self {
            self.x *= other.x;
            self.y *= other.y;
            self.z *= other.z;
            self.w *= other.w;
            return self;
        }

        pub fn div(self: Self, other: Self) Self {
            return .{
                .x = self.x / other.x,
                .y = self.y / other.y,
                .z = self.z / other.z,
                .w = self.w / other.w,
            };
        }

        pub fn divMut(self: *Self, other: Self) *Self {
            self.x /= other.x;
            self.y /= other.y;
            self.z /= other.z;
            self.w /= other.w;
            return self;
        }

        pub fn xyz(self: Self) Vec3(T) {
            return .init(self.x, self.y, self.z);
        }
    };
}
