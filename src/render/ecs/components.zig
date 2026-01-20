const render = @import("../root.zig");

const Meshes = render.Meshes;

pub const Mesh = struct {
    handle: Meshes.Mesh.Handle = .invalid,
};
