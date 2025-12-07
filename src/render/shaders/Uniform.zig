const core = @import("core");
const zbgfx = @import("zbgfx");

const Mat = core.math.Mat;
const Vec = core.math.Vec;

const Self = @This();

handle: zbgfx.bgfx.UniformHandle,

pub fn setVec(self: Self, value: Vec) void {
    zbgfx.bgfx.setUniform(self.handle, @ptrCast(&value.toArray()), 1);
}

pub fn setArray(self: Self, value: []const f32) void {
    zbgfx.bgfx.setUniform(self.handle, @ptrCast(value), 1);
}

pub fn setMat(self: Self, value: Mat) void {
    zbgfx.bgfx.setUniform(self.handle, @ptrCast(&value.toArray()), 1);
}
