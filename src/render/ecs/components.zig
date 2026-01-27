const core = @import("core");
const render = @import("../root.zig");

const materials = render.materials;
const Meshes = render.Meshes;
const Program = render.shaders.Program;
const Texture = render.Texture;
const Vec = core.math.Vec;

pub const Mesh = struct {
    handle: Meshes.Mesh.Handle = .invalid,
};

pub const Phong = struct {
    diffuse: Texture = .{},
    diffuse_color: Vec = .splat(1.0),
    specular: Texture = .{},
    specular_color: Vec = .zero,
    shininess: f32 = 32.0,
};

pub const Light = struct {
    position: Vec = .zero,
    ambient: Vec = .splat(1.0),
    diffuse: Vec = .splat(1.0),
    specular: Vec = .splat(1.0),
};

pub const Color = struct {
    tint: Vec = .splat(1.0),
};
