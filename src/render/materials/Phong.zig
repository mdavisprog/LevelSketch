const core = @import("core");
const render = @import("../root.zig");

const Vec = core.math.Vec;

const Program = render.shaders.Program;

const Self = @This();

ambient: Vec = .zero,
diffuse: Vec = .zero,
specular: Vec = .zero,
shininess: f32 = 32.0,

pub fn bind(self: Self, shader: *const Program) !void {
    try shader.setUniform("u_ambient", self.ambient);
    try shader.setUniform("u_diffuse", self.diffuse);
    try shader.setUniform("u_specular_shininess", Vec.init(
        self.specular.x(),
        self.specular.y(),
        self.specular.z(),
        self.shininess,
    ));
}
