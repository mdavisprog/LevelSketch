const rect = @import("rect.zig");
const vec2 = @import("vec2.zig");
const vec3 = @import("vec3.zig");

pub const Recti = rect.rect(i32);
pub const Rectf = rect.rect(f32);
pub const Rectus = rect.rect(usize);

pub const Vec2i = vec2.Vec2(i32);
pub const Vec2f = vec2.Vec2(f32);
pub const Vec2us = vec2.Vec2(usize);

pub const Vec3i = vec3.Vec3(i32);
pub const Vec3f = vec3.Vec3(f32);
