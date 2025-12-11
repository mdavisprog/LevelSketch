const clay = @import("clay");
const core = @import("core");
const render = @import("render");
const std = @import("std");
const zbgfx = @import("zbgfx");

const HexColor = core.math.HexColor;
const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;

const Commands = render.Commands;
const Fonts = render.Fonts;
const RenderBuffer = render.RenderBuffer;
const Renderer = render.Renderer;
const VertexBuffer16 = render.VertexBuffer16;

/// Object that manages layout setup and translating the clay render commands to the internal
/// commands.
const Self = @This();

// TODO: Should this be injected instead of holding a reference?
renderer: *Renderer,

pub fn init(renderer: *Renderer) Self {
    clay.setMeasureTextFunction(onMeasureText, @ptrCast(renderer.fonts));
    return .{
        .renderer = renderer,
    };
}

pub fn begin(self: Self) void {
    clay.setLayoutDimensions(toDimensions(self.renderer.framebuffer_size));
    clay.builder.begin();
}

pub fn end(self: Self, commands: *Commands) !void {
    const render_commands = clay.builder.end();
    try renderCommands(self.renderer, render_commands.slice(), commands);
}

fn onMeasureText(
    text: clay.StringSlice,
    config: [*c]clay.TextElementConfig,
    user_data: ?*anyopaque,
) callconv(.c) clay.Dimensions {
    const fonts: *Fonts = @ptrCast(@alignCast(user_data.?));
    const font = fonts.getById(config.*.font_id);
    const size: Vec2f = if (font) |f| f.measure(text.str()) else .zero;

    return toDimensions(size);
}

fn renderCommands(
    renderer: *Renderer,
    render_commands: []const clay.RenderCommand,
    commands: *Commands,
) !void {
    for (render_commands) |command| {
        try renderCommand(renderer, command, commands);
    }
}

fn renderCommand(
    renderer: *Renderer,
    render_command: clay.RenderCommand,
    commands: *Commands,
) !void {
    const rect = rectFromBoundingBox(render_command.bounding_box);
    switch (render_command.command_type) {
        .rectangle => {
            const corner_radius = render_command.render_data.rectangle.corner_radius;
            const color = hexColor(render_command.render_data.rectangle.background_color);

            var quad = if (corner_radius.isZero())
                try render.shapes.quad(u16, renderer.allocator, rect, color.data)
            else
                try render.shapes.quadRounded(
                    u16,
                    renderer.allocator,
                    rect,
                    color.data,
                    corner_radius.toArray(),
                );
            defer quad.deinit(renderer.allocator);

            var render_buffer: RenderBuffer = .init();
            try render_buffer.setTransientBuffer(quad);

            const shader = try renderer.programs.get("common");
            try commands.addCommand(.{
                .buffer = render_buffer,
                .texture = renderer.textures.default,
                .texture_flags = 0,
                .shader = shader,
                .sampler = try shader.getUniform("s_tex_color"),
                .state = Renderer.ui_state,
            });
        },
        .text => {
            const text_data = render_command.render_data.text;
            const color = hexColor(text_data.text_color);
            const maybe_font = renderer.fonts.getById(text_data.font_id);

            var buffer = blk: {
                if (maybe_font) |font| {
                    break :blk try font.getVertices(
                        renderer.allocator,
                        text_data.string_contents.str(),
                        rect.min,
                    );
                } else {
                    break :blk try render.shapes.quad(u16, renderer.allocator, rect, color.data);
                }
            };
            defer buffer.deinit(renderer.allocator);

            var render_buffer: RenderBuffer = .init();
            try render_buffer.setTransientBuffer(buffer);

            const shader = try renderer.programs.get("text");
            try commands.addCommand(.{
                .buffer = render_buffer,
                .texture = if (maybe_font) |font| font.texture else .{},
                .texture_flags = zbgfx.bgfx.SamplerFlags_UBorder | zbgfx.bgfx.SamplerFlags_VBorder,
                .shader = shader,
                .sampler = try shader.getUniform("s_font"),
                .state = Renderer.ui_state,
            });
        },
        else => {
            std.debug.print("Unhandled clay render command type: {}\n", .{render_command.command_type});
        },
    }
}

fn rectFromBoundingBox(bbox: clay.BoundingBox) Rectf {
    return .init(
        bbox.x,
        bbox.y,
        bbox.x + bbox.width,
        bbox.y + bbox.height,
    );
}

fn hexColor(color: clay.Color) HexColor {
    return .init(
        @intFromFloat(color.r),
        @intFromFloat(color.g),
        @intFromFloat(color.b),
        @intFromFloat(color.a),
        HexColor.Format.abgr,
    );
}

fn toDimensions(value: Vec2f) clay.Dimensions {
    return .init(value.x, value.y);
}
