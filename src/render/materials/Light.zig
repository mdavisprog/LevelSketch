const core = @import("core");
const render = @import("../root.zig");

const Vec = core.math.Vec;

const Program = render.shaders.Program;

const Self = @This();

position: Vec = .zero,
ambient: Vec = .zero,
diffuse: Vec = .zero,
specular: Vec = .zero,

pub fn bind(self: Self, shader: *const Program) !void {
    try self.bindPosition(shader);
    try shader.setUniform("u_light_ambient", self.ambient);
    try shader.setUniform("u_light_diffuse", self.diffuse);
    try shader.setUniform("u_light_specular", self.specular);
}

pub fn bindPosition(self: Self, shader: *const Program) !void {
    try shader.setUniform("u_light_position", self.position);
}
