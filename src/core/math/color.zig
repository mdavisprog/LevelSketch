const Vec = @import("Vec.zig");

pub fn Color4(comptime T: type) type {
    if (T != u8 and T != f32) {
        @compileError("Color4 can only be of type u8 or f32.");
    }

    return struct {
        const Self = @This();

        pub const white: Self = .make(255, 255, 255, 255);
        pub const black: Self = .make(0, 0, 0, 255);
        pub const transparent: Self = .make(0, 0, 0, 0);

        r: T = 0,
        g: T = 0,
        b: T = 0,
        a: T = 0,

        pub fn init(r: T, g: T, b: T, a: T) Self {
            return .{
                .r = r,
                .g = g,
                .b = b,
                .a = a,
            };
        }

        pub fn toVec(self: Self) Vec {
            if (T == f32) {
                return .init(self.r, self.g, self.b, self.a);
            } else {
                const r = @as(f32, @floatFromInt(self.r)) / 255.0;
                const g = @as(f32, @floatFromInt(self.g)) / 255.0;
                const b = @as(f32, @floatFromInt(self.b)) / 255.0;
                const a = @as(f32, @floatFromInt(self.a)) / 255.0;
                return .init(r, g, b, a);
            }
        }

        fn make(comptime r: u8, comptime g: u8, comptime b: u8, comptime a: u8) Self {
            if (T == f32) {
                return .{
                    .r = @as(f32, @floatFromInt(r)) / 255.0,
                    .g = @as(f32, @floatFromInt(g)) / 255.0,
                    .b = @as(f32, @floatFromInt(b)) / 255.0,
                    .a = @as(f32, @floatFromInt(a)) / 255.0,
                };
            } else {
                return .{
                    .r = r,
                    .g = g,
                    .b = b,
                    .a = a,
                };
            }
        }
    };
}
