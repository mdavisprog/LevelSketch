const render = @import("../root.zig");

const Renderer = render.Renderer;

pub const Render = struct {
    renderer: *Renderer,
};
