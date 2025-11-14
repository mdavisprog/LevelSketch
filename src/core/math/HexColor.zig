const math = @import("root.zig");
const std = @import("std");

const Color4b = math.Color4b;

/// Allow for conversion between Color4b and u32.
const Self = @This();

pub const Format = enum(u8) {
    rgba,
    abgr,
};

data: u32 = 0,
format: Format = .rgba,

pub fn init(r: u8, g: u8, b: u8, a: u8, format: Format) Self {
    return .{
        .data = encode(r, g, b, a, format),
        .format = format,
    };
}

pub fn from(color: Color4b, format: Format) Self {
    return .{
        .data = encode(
            color.r,
            color.g,
            color.b,
            color.a,
            format,
        ),
        .format = format,
    };
}

fn encode(r: u8, g: u8, b: u8, a: u8, format: Format) u32 {
    const _r: u32 = @intCast(r);
    const _g: u32 = @intCast(g);
    const _b: u32 = @intCast(b);
    const _a: u32 = @intCast(a);

    var result: u32 = 0;
    switch (format) {
        .abgr => {
            result |= _r;
            result |= std.math.shl(u32, _g, 8);
            result |= std.math.shl(u32, _b, 16);
            result |= std.math.shl(u32, _a, 24);
        },
        .rgba => {
            result |= std.math.shl(u32, _r, 24);
            result |= std.math.shl(u32, _g, 16);
            result |= std.math.shl(u32, _b, 8);
            result |= _a;
        },
    }

    return result;
}

test "encode" {
    const hex: Self = .init(255, 0, 0, 255, .rgba);
    try std.testing.expectEqual(0xFF0000FF, hex.data);

    const hex2: Self = .init(0, 255, 0, 255, .abgr);
    try std.testing.expectEqual(0xFF00FF00, hex2.data);
}
