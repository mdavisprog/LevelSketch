const core = @import("core");
const render = @import("../root.zig");

const Vec = core.math.Vec;

const Program = render.shaders.Program;
const Texture = render.Texture;

const Self = @This();

diffuse: Texture = .{},
specular: Texture = .{},
specular_color: Vec = .zero,
shininess: f32 = 32.0,

pub fn bind(self: Self, shader: *const Program) !void {
    try shader.setTexture("s_diffuse", self.diffuse, 0);
    try shader.setTexture("s_specular", self.specular, 1);
    try shader.setUniform("u_specular_shininess", Vec.init(
        self.specular_color.x(),
        self.specular_color.y(),
        self.specular_color.z(),
        self.shininess,
    ));
}
