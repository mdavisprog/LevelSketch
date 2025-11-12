const core = @import("core");
const render = @import("render");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;

const Font = render.Font;
const MemFactory = render.MemFactory;
const Program = render.shaders.Program;
const RenderBuffer = render.RenderBuffer;
const Textures = render.Textures;
const VertexBuffer16 = render.VertexBuffer16;
const View = render.View;

const Self = @This();

view: View,
size: Vec2f = .zero,
font: Font,
text_shader: Program,
_buffer: RenderBuffer,

/// Must be initialized after bgfx has been initialized.
pub fn init(factory: *MemFactory, textures: *Textures) !Self {
    var view: View = .init(0x000000FF, false);
    view.setMode(.Sequential);

    const font: Font = try .init(
        factory,
        "assets/fonts/Roboto-Regular.ttf",
        32.0,
        textures,
    );

    var text_shader: Program = .{};
    _ = try text_shader.build(
        factory.allocator,
        .{
            .varying_file_name = "common.def.sc",
            .fragment_file_name = "text_fragment.sc",
            .vertex_file_name = "common_vertex.sc",
        },
    );

    var text_buffer = try font.getVertices(factory.allocator, "Hello World", .init(50.0, 50.0));
    defer text_buffer.deinit(factory.allocator);

    var buffer: RenderBuffer = .init();
    try buffer.setTransientVertices(
        try text_buffer.createMemVertexTransient(factory),
        text_buffer.vertices.items.len,
    );

    try buffer.setTransientIndices(
        try text_buffer.createMemIndexTransient(factory),
        text_buffer.indices.items.len,
    );

    return Self{
        .view = view,
        .font = font,
        .text_shader = text_shader,
        ._buffer = buffer,
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self.text_shader.clean();
    self.font.deinit(allocator);
    self._buffer.deinit();
}

pub fn setView(self: *Self, width: f32, height: f32) void {
    self.view.setOrthographic(width, height);
    self.size = .init(width, height);
}

pub fn update(self: *Self, delta_time: f32) void {
    _ = self;
    _ = delta_time;
}

pub fn draw(
    self: Self,
    sampler: zbgfx.bgfx.UniformHandle,
    default_shader: Program,
) !void {
    if (!self._buffer.isValid()) {
        return;
    }

    const state = zbgfx.bgfx.StateFlags_WriteRgb |
        zbgfx.bgfx.StateFlags_WriteA |
        zbgfx.bgfx.StateFlags_Msaa |
        render.stateFlagsBlend(
            zbgfx.bgfx.StateFlags_BlendSrcAlpha,
            zbgfx.bgfx.StateFlags_BlendInvSrcAlpha,
        );

    _ = default_shader;

    const width: u16 = @intFromFloat(self.size.x);
    const height: u16 = @intFromFloat(self.size.y);
    self.view.submitOrthographic(width, height);

    try self.font.texture.bind(
        sampler,
        zbgfx.bgfx.SamplerFlags_UBorder | zbgfx.bgfx.SamplerFlags_VBorder,
    );
    self._buffer.bind(state);
    zbgfx.bgfx.submit(self.view.id, self.text_shader.handle, 255, 0);
    self.view.touch();
}
