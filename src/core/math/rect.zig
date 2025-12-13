const vec2 = @import("vec2.zig");

pub fn rect(comptime T: type) type {
    return struct {
        const Self = @This();
        const TVec = vec2.Vec2(T);

        pub const zero: Self = .init(0, 0, 0, 0);

        min: TVec = TVec.zero,
        max: TVec = TVec.zero,

        pub fn init(min_x: T, min_y: T, max_x: T, max_y: T) Self {
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

        pub fn contains(self: Self, point: TVec) bool {
            return self.min.x <= point.x and point.x <= self.max.x and
                self.min.y <= point.y and point.y <= self.max.y;
        }

        pub fn move(self: *Self, delta: TVec) void {
            _ = self.min.addMut(delta);
            _ = self.max.addMut(delta);
        }
    };
}
