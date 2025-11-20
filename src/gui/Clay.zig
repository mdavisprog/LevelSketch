const clay = @import("clay");
const core = @import("core");
const render = @import("render");
const std = @import("std");
const zbgfx = @import("zbgfx");

const HexColor = core.math.HexColor;
const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;

const Commands = render.Commands;
const Font = render.Font;
const Fonts = render.Fonts;
const RenderBuffer = render.RenderBuffer;
const Renderer = render.Renderer;
const VertexBuffer16 = render.VertexBuffer16;

const Self = @This();

_memory: ?[]const u8 = null,
_arena: clay.Arena = .{},
_context: ?*clay.Context = null,

pub fn init(renderer: *const Renderer) !Self {
    const min_size = clay.minMemorySize();
    const memory = try renderer._gpa.alloc(u8, min_size);

    const bytes: f32 = @floatFromInt(min_size);
    const mb: f32 = bytes / 1024.0 / 1024.0;

    const arena = clay.createArenaWithCapacityAndMemory(
        min_size,
        @ptrCast(memory.ptr),
    );

    std.log.info("Clay arena memory size: {d:.2}MB", .{mb});

    const dimensions = toDimensions(renderer.framebuffer_size);
    const context = clay.initialize(arena, dimensions, .{
        .error_handler_function = onError,
    });

    clay.setMeasureTextFunction(onMeasureText, @ptrCast(renderer.fonts));

    return Self{
        ._memory = memory,
        ._arena = arena,
        ._context = context,
    };
}

pub fn deinit(self: *Self, gpa: std.mem.Allocator) void {
    if (self._memory) |memory| {
        gpa.free(memory);
        self._memory = null;
    }
}

pub fn build(renderer: *Renderer, commands: *Commands) !void {
    clay.setLayoutDimensions(toDimensions(renderer.framebuffer_size));
    clay.builder.begin();
    {
        clay.builder.beginElement("Test", .{
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
            clay.builder.beginElement("Inner", .{
                .layout = .{
                    .sizing = .fixed(50, 50),
                },
                .background_color = .initu8(180, 70, 70, 255),
            });
            defer clay.builder.endElement();
            {
                clay.builder.beginTextElement("Hello", .{
                    .font_id = renderer.fonts.default,
                });
                clay.builder.endElement();
            }
        }
    }
    const render_commands = clay.builder.end();

    try renderCommands(renderer, render_commands.slice(), commands);
}

fn onError(error_data: clay.ErrorData) callconv(.c) void {
    std.log.warn(
        "Clay Error: {} Message: {s}",
        .{ error_data.error_type, error_data.error_text.str() },
    );
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
                try render.shapes.quad(renderer._gpa, rect, color.data)
            else
                try render.shapes.quadRounded(
                    renderer._gpa,
                    rect,
                    color.data,
                    corner_radius.toArray(),
                );
            defer quad.deinit(renderer._gpa);

            var render_buffer: RenderBuffer = .init();
            try render_buffer.setTransientBuffer(&renderer.mem_factory, quad);

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

            var buffer: VertexBuffer16 = blk: {
                if (maybe_font) |font| {
                    break :blk try font.getVertices(
                        renderer._gpa,
                        text_data.string_contents.str(),
                        rect.min,
                    );
                } else {
                    break :blk try render.shapes.quad(renderer._gpa, rect, color.data);
                }
            };
            defer buffer.deinit(renderer._gpa);

            var render_buffer: RenderBuffer = .init();
            try render_buffer.setTransientBuffer(&renderer.mem_factory, buffer);

            const shader = try renderer.programs.get("text");
            try commands.addCommand(.{
                .buffer = render_buffer,
                .texture = if (maybe_font) |font| font.texture else .{},
                .texture_flags = zbgfx.bgfx.SamplerFlags_UBorder | zbgfx.bgfx.SamplerFlags_VBorder,
                .shader = shader,
                .sampler = try shader.getUniform("s_tex_color"),
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
