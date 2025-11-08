pub fn Vec3(comptime T: type) type {
    return struct {
        const Self = @This();

        pub const ZERO: Self = .{
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
    };
}
