const color = @import("color.zig");
const rect = @import("rect.zig");
const std = @import("std");
const vec2 = @import("vec2.zig");
const vec3 = @import("vec3.zig");
const vec4 = @import("vec4.zig");

pub const Color4b = color.Color4(u8);
pub const Color4f = color.Color4(f32);
pub const HexColor = @import("HexColor.zig");

pub const Recti = rect.rect(i32);
pub const Rectf = rect.rect(f32);
pub const Rectus = rect.rect(usize);

pub const Vec2i = vec2.Vec2(i32);
pub const Vec2f = vec2.Vec2(f32);
pub const Vec2us = vec2.Vec2(usize);

pub const Vec3i = vec3.Vec3(i32);
pub const Vec3f = vec3.Vec3(f32);

pub const Vec4f = vec4.Vec4(f32);

test "refall" {
    std.testing.refAllDecls(@This());
}
