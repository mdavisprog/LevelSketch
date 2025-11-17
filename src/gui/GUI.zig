const Clay = @import("Clay.zig");
const core = @import("core");
const render = @import("render");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Color4b = core.math.Color4b;
const HexColor = core.math.HexColor;
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

const hover_color: u32 = 0xFF00FF00;
const unhover_color: u32 = 0xFF0000FF;

view: View,
size: Vec2f = .zero,
font: Font,
text_shader: *Program,
common_shader: *Program,
clay: Clay = .{},
_commands: Commands,
_default_texture: Texture,
_test_rect: Rectf = .init(400.0, 50.0, 500.0, 100.0),
_test_rect_hovered: bool = false,

/// Must be initialized after bgfx has been initialized.
pub fn init(renderer: *Renderer) !Self {
    var view: View = .init(0x000000FF, false);
    view.setMode(.Sequential);

    const font: Font = try .init(
        renderer,
        "assets/fonts/Roboto-Regular.ttf",
        32.0,
    );

    const text_shader: *Program = try renderer.programs.build(
        renderer._gpa,
        "text",
        .{
            .varying_file_name = "common.def.sc",
            .fragment_file_name = "text_fragment.sc",
            .vertex_file_name = "common_vertex.sc",
        },
    );

    const clay: Clay = try .init(renderer._gpa);

    var result = Self{
        .view = view,
        .font = font,
        .text_shader = text_shader,
        .common_shader = try renderer.programs.get("common"),
        ._commands = try Commands.init(renderer.mem_factory.allocator),
        ._default_texture = renderer.textures.default,
        .clay = clay,
    };

    try result.buildCommands(renderer, unhover_color);

    return result;
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self.clay.deinit(allocator);
    self.font.deinit(allocator);
    self._commands.deinit();
}

pub fn setView(self: *Self, width: f32, height: f32) void {
    self.view.setOrthographic(width, height);
    self.size = .init(width, height);
}

pub fn update(self: *Self, renderer: *Renderer, delta_time: f32, cursor_pos: Vec2f) !void {
    _ = delta_time;

    if (self._test_rect.contains(cursor_pos)) {
        if (!self._test_rect_hovered) {
            self._test_rect_hovered = true;
            try self.buildCommands(renderer, hover_color);
        }
    } else {
        if (self._test_rect_hovered) {
            self._test_rect_hovered = false;
            try self.buildCommands(renderer, unhover_color);
        }
    }
}

pub fn draw(self: *Self) !void {
    const width: u16 = @intFromFloat(self.size.x);
    const height: u16 = @intFromFloat(self.size.y);
    self.view.submitOrthographic(width, height);

    try self._commands.run(self.view);
}

fn buildCommands(self: *Self, renderer: *Renderer, color: u32) !void {
    const allocator = renderer.mem_factory.allocator;

    self._commands.clear();

    var text_buffer = try self.font.getVertices(allocator, "Hello World", .init(50.0, 50.0));
    defer text_buffer.deinit(allocator);

    var buffer: RenderBuffer = .init();
    try buffer.setTransientBuffer(&renderer.mem_factory, text_buffer);

    try self._commands.addCommand(.{
        .buffer = buffer,
        .texture = self.font.texture,
        .texture_flags = zbgfx.bgfx.SamplerFlags_UBorder | zbgfx.bgfx.SamplerFlags_VBorder,
        .shader = self.text_shader,
        .sampler = try self.text_shader.getUniform("s_tex_color"),
        .state = Renderer.ui_state,
    });

    var quad = try render.shapes.quad(allocator, self._test_rect, color);
    defer quad.deinit(allocator);

    var quad_buffer: RenderBuffer = .init();
    try quad_buffer.setTransientBuffer(&renderer.mem_factory, quad);

    try self._commands.addCommand(.{
        .buffer = quad_buffer,
        .texture = self._default_texture,
        .texture_flags = 0,
        .shader = self.common_shader,
        .sampler = try self.common_shader.getUniform("s_tex_color"),
        .state = Renderer.ui_state,
    });

    try Clay.build(renderer, self.size, &self._commands);
}
