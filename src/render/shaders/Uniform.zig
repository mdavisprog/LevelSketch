const core = @import("core");
const render = @import("../root.zig");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Mat = core.math.Mat;
const Vec = core.math.Vec;

const Texture = render.Texture;

const Self = @This();

handle: zbgfx.bgfx.UniformHandle,

pub fn set(self: Self, value: anytype) void {
    const ValueType = @TypeOf(value);
    switch (ValueType) {
        Vec => self.setVec(value),
        Mat => self.setMat(value),
        f32 => self.setFloat(value),
        []const f32 => self.setArray(value),
        Texture => self.setTexture(value),
        else => @compileError(std.fmt.comptimePrint(
            "Invalid type {s}. Must be of type Vec, Mat, or []const f32",
            .{@typeName(ValueType)},
        )),
    }
}

pub fn setVec(self: Self, value: Vec) void {
    zbgfx.bgfx.setUniform(self.handle, @ptrCast(&value.toArray()), 1);
}

pub fn setArray(self: Self, value: []const f32) void {
    zbgfx.bgfx.setUniform(self.handle, @ptrCast(value), 1);
}

pub fn setMat(self: Self, value: Mat) void {
    zbgfx.bgfx.setUniform(self.handle, @ptrCast(&value.toArray()), 1);
}

pub fn setTexture(self: Self, texture: Texture) void {
    const handle = texture.handle orelse return;
    zbgfx.bgfx.setTexture(0, self.handle, handle, texture.flags);
}
