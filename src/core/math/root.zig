const color = @import("color.zig");
const rect = @import("rect.zig");
const std = @import("std");
const vec2 = @import("vec2.zig");

pub const Color4b = color.Color4(u8);
pub const Color4f = color.Color4(f32);
pub const HexColor = @import("HexColor.zig");

pub const Recti = rect.rect(i32);
pub const Rectf = rect.rect(f32);
pub const Rectus = rect.rect(usize);

pub const Vec2i = vec2.Vec2(i32);
pub const Vec2f = vec2.Vec2(f32);
pub const Vec2us = vec2.Vec2(usize);

pub const Vec = @import("Vec.zig");

test "refall" {
    std.testing.refAllDecls(@This());
}
