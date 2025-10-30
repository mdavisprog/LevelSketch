pub fn Vec2(comptime T: type) type {
    return struct {
        const Self = @This();

        x: T = 0,
        y: T = 0,

        pub fn init(x: T, y: T) Self {
            return Self{
                .x = x,
                .y = y,
            };
        }
    };
}
