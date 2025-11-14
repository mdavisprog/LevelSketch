const core = @import("core");
const render = @import("render");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;

const Commands = render.Commands;
const Font = render.Font;
const MemFactory = render.MemFactory;
const Program = render.shaders.Program;
const RenderBuffer = render.RenderBuffer;
const Renderer = render.Renderer;
const Texture = render.Texture;
const Textures = render.Textures;
const Uniform = render.shaders.Uniform;
const VertexBuffer16 = render.VertexBuffer16;
const View = render.View;

const Self = @This();

view: View,
size: Vec2f = .zero,
font: Font,
text_shader: Program,
_commands: Commands,
_default_texture: Texture,

/// Must be initialized after bgfx has been initialized.
pub fn init(renderer: *Renderer) !Self {
    var view: View = .init(0x000000FF, false);
    view.setMode(.Sequential);

    const font: Font = try .init(
        renderer,
        "assets/fonts/Roboto-Regular.ttf",
        32.0,
    );

    var text_shader: Program = .init(renderer.mem_factory.allocator);
    try text_shader.build(
        .{
            .varying_file_name = "common.def.sc",
            .fragment_file_name = "text_fragment.sc",
            .vertex_file_name = "common_vertex.sc",
        },
    );

    var result = Self{
        .view = view,
        .font = font,
        .text_shader = text_shader,
        ._commands = try Commands.init(renderer.mem_factory.allocator),
        ._default_texture = renderer.textures.default,
    };

    try result.buildCommands(&renderer.mem_factory);

    return result;
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self.text_shader.deinit();
    self.font.deinit(allocator);
    self._commands.deinit();
}

pub fn setView(self: *Self, width: f32, height: f32) void {
    self.view.setOrthographic(width, height);
    self.size = .init(width, height);
}

pub fn update(self: *Self, delta_time: f32) void {
    _ = self;
    _ = delta_time;
}

pub fn draw(self: *Self) !void {
    const width: u16 = @intFromFloat(self.size.x);
    const height: u16 = @intFromFloat(self.size.y);
    self.view.submitOrthographic(width, height);

    try self._commands.run(self.view);
}

fn buildCommands(self: *Self, factory: *MemFactory) !void {
    const allocator = factory.allocator;

    self._commands.clear();

    const state = zbgfx.bgfx.StateFlags_WriteRgb |
        zbgfx.bgfx.StateFlags_WriteA |
        zbgfx.bgfx.StateFlags_Msaa |
        render.stateFlagsBlend(
            zbgfx.bgfx.StateFlags_BlendSrcAlpha,
            zbgfx.bgfx.StateFlags_BlendInvSrcAlpha,
        );

    var text_buffer = try self.font.getVertices(allocator, "Hello World", .init(50.0, 50.0));
    defer text_buffer.deinit(allocator);

    var buffer: RenderBuffer = .init();
    try buffer.setTransientBuffer(factory, text_buffer);

    try self._commands.addCommand(.{
        .buffer = buffer,
        .texture = self.font.texture,
        .texture_flags = zbgfx.bgfx.SamplerFlags_UBorder | zbgfx.bgfx.SamplerFlags_VBorder,
        .shader = self.text_shader,
        .sampler = try self.text_shader.getUniform("s_tex_color"),
        .state = state,
    });
}
