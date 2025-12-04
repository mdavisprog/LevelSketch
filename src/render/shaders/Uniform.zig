const core = @import("core");
const zbgfx = @import("zbgfx");
const zmath = @import("zmath");

const Vec4f = core.math.Vec4f;

const Self = @This();

handle: zbgfx.bgfx.UniformHandle,

pub fn setVec4f(self: Self, value: Vec4f) void {
    zbgfx.bgfx.setUniform(self.handle, @ptrCast(&value.toArray()), 1);
}

pub fn setArray(self: Self, value: []const f32) void {
    zbgfx.bgfx.setUniform(self.handle, @ptrCast(value), 1);
}

pub fn setMat(self: Self, value: zmath.Mat) void {
    zbgfx.bgfx.setUniform(self.handle, @ptrCast(&zmath.matToArr(value)), 1);
}
