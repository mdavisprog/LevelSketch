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
    };
}
