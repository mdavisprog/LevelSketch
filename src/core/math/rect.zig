const vec2 = @import("vec2.zig");

pub fn rect(comptime T: type) type {
    return struct {
        const Self = @This();
        const TVec = vec2.Vec2(T);

        min: TVec = TVec.ZERO,
        max: TVec = TVec.ZERO,

        pub fn init(min_x: usize, min_y: usize, max_x: usize, max_y: usize) Self {
            return Self{
                .min = .{ .x = min_x, .y = min_y },
                .max = .{ .x = max_x, .y = max_y },
            };
        }

        pub fn width(self: Self) T {
            return self.max.x - self.min.x;
        }

        pub fn height(self: Self) T {
            return self.max.y - self.min.y;
        }
    };
}
