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

view: View,
font: *Font,
text_shader: *Program,
common_shader: *Program,
clay: Clay = .{},
_commands: Commands,
_default_texture: Texture,

/// Must be initialized after bgfx has been initialized.
pub fn init(renderer: *Renderer) !Self {
    const framebuffer_size = renderer.framebuffer_size;

    var view: View = .init(0x000000FF, false);
    view.setMode(.Sequential);
    view.setOrthographic(framebuffer_size.x, framebuffer_size.y);

    const font = try renderer.fonts.loadFile(
        renderer,
        "Roboto-Regular.ttf",
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

    const clay: Clay = try .init(renderer);

    var result = Self{
        .view = view,
        .font = font,
        .text_shader = text_shader,
        .common_shader = try renderer.programs.get("common"),
        ._commands = try Commands.init(renderer.mem_factory.allocator),
        ._default_texture = renderer.textures.default,
        .clay = clay,
    };

    try result.buildCommands(renderer);

    return result;
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self.clay.deinit(allocator);
    self._commands.deinit();
}

pub fn update(self: *Self, renderer: *Renderer, delta_time: f32, cursor_pos: Vec2f) !void {
    _ = self;
    _ = renderer;
    _ = delta_time;
    _ = cursor_pos;
}

pub fn draw(self: *Self, renderer: *const Renderer) !void {
    const width: u16 = @intFromFloat(renderer.framebuffer_size.x);
    const height: u16 = @intFromFloat(renderer.framebuffer_size.y);
    self.view.submitOrthographic(width, height);

    try self._commands.run(self.view);
}

fn buildCommands(self: *Self, renderer: *Renderer) !void {
    self._commands.clear();
    try Clay.build(renderer, &self._commands);
}
