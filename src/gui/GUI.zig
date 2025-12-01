const clay = @import("clay");
const ClayContext = @import("ClayContext.zig");
const ClayLayout = @import("ClayLayout.zig");
const core = @import("core");
const render = @import("render");
const std = @import("std");

const Vec2f = core.math.Vec2f;

const Commands = render.Commands;
const Font = render.Font;
const Program = render.shaders.Program;
const Renderer = render.Renderer;
const Texture = render.Texture;
const View = render.View;

const Self = @This();

view: View,
font: *Font,
text_shader: *Program,
common_shader: *Program,
clay_context: ClayContext,
_clay_layout: ClayLayout,
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
        renderer.allocator,
        "text",
        .{
            .varying_file_name = "common.def.sc",
            .fragment_file_name = "text_fragment.sc",
            .vertex_file_name = "common_vertex.sc",
        },
    );

    const clay_context: ClayContext = try .init(renderer.allocator);

    var result = Self{
        .view = view,
        .font = font,
        .text_shader = text_shader,
        .common_shader = try renderer.programs.get("common"),
        ._commands = try Commands.init(renderer.mem_factory.allocator),
        ._default_texture = renderer.textures.default,
        .clay_context = clay_context,
        ._clay_layout = .init(renderer),
    };

    try result.layout();

    return result;
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self.clay_context.deinit(allocator);
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

fn layout(self: *Self) !void {
    self._commands.clear();

    self._clay_layout.begin();
    {
        clay.builder.beginElementId("Test", .{
            .layout = .{
                .padding = .splat(5),
                .sizing = .fixed(200, 200),
            },
            .background_color = .initu8(70, 70, 190, 255),
            .corner_radius = .all(5.0),
            .floating = .{
                .offset = .init(20.0, 20.0),
                .attach_to = .root,
            },
        });
        defer clay.builder.endElement();
        {
            clay.builder.beginElementId("Inner", .{
                .layout = .{
                    .sizing = .fixed(50, 50),
                },
                .background_color = .initu8(180, 70, 70, 255),
            });
            defer clay.builder.endElement();
            {
                clay.builder.textElement("Hello", .{
                    .font_id = self._clay_layout.renderer.fonts.default,
                });
            }
        }
    }
    try self._clay_layout.end(&self._commands);
}
