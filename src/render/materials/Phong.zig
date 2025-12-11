const core = @import("core");
const render = @import("../root.zig");

const Vec = core.math.Vec;

const Program = render.shaders.Program;
const Texture = render.Texture;

const Self = @This();

diffuse: Texture = .{},
specular: Vec = .zero,
shininess: f32 = 32.0,

pub fn bind(self: Self, shader: *const Program) !void {
    try shader.setUniform("s_diffuse", self.diffuse);
    try shader.setUniform("u_specular_shininess", Vec.init(
        self.specular.x(),
        self.specular.y(),
        self.specular.z(),
        self.shininess,
    ));
}
