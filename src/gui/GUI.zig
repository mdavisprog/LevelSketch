const clay = @import("clay");
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
_commands: Commands,
_default_texture: Texture,
_test_rect: Rectf = .init(400.0, 50.0, 500.0, 100.0),
_test_rect_hovered: bool = false,
_clay_memory: ?[]u8 = null,
_clay_arena: clay.Arena = .{},
_clay_context: ?*clay.Context = null,

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

    var result = Self{
        .view = view,
        .font = font,
        .text_shader = text_shader,
        .common_shader = try renderer.programs.get("common"),
        ._commands = try Commands.init(renderer.mem_factory.allocator),
        ._default_texture = renderer.textures.default,
    };

    try result.initClay(renderer._gpa);
    try result.buildCommands(&renderer.mem_factory, unhover_color);

    return result;
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    if (self._clay_memory) |memory| {
        allocator.free(memory);
        self._clay_memory = null;
    }

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
            try self.buildCommands(&renderer.mem_factory, hover_color);
        }
    } else {
        if (self._test_rect_hovered) {
            self._test_rect_hovered = false;
            try self.buildCommands(&renderer.mem_factory, unhover_color);
        }
    }
}

pub fn draw(self: *Self) !void {
    const width: u16 = @intFromFloat(self.size.x);
    const height: u16 = @intFromFloat(self.size.y);
    self.view.submitOrthographic(width, height);

    try self._commands.run(self.view);
}

fn buildCommands(self: *Self, factory: *MemFactory, color: u32) !void {
    const allocator = factory.allocator;

    self._commands.clear();

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
        .state = Renderer.ui_state,
    });

    var quad = try render.shapes.quad(allocator, self._test_rect, color);
    defer quad.deinit(allocator);

    var quad_buffer: RenderBuffer = .init();
    try quad_buffer.setTransientBuffer(factory, quad);

    try self._commands.addCommand(.{
        .buffer = quad_buffer,
        .texture = self._default_texture,
        .texture_flags = 0,
        .shader = self.common_shader,
        .sampler = try self.common_shader.getUniform("s_tex_color"),
        .state = Renderer.ui_state,
    });

    const layout: clay.LayoutConfig = .{ .padding = .splat(5), .sizing = .fixed(50, 50) };

    const inner: clay.LayoutConfig = .{
        .sizing = .{
            .width = .fixed(20),
            .height = .fixed(20),
        },
    };

    clay.beginLayout();
    clay.openElement();
    clay.configureOpenElement(.{
        .id = clay.id("Test"),
        .layout = layout,
        .background_color = .initu8(0, 255, 0, 255),
    });

    clay.openElement();
    clay.configureOpenElement(.{
        .id = clay.id("Inner"),
        .layout = inner,
        .background_color = .initu8(200, 50, 50, 255),
    });
    clay.closeElement();

    clay.closeElement();
    const render_commands = clay.endLayout();

    const clay_buffers = try renderClayCommands(allocator, render_commands.slice());
    defer {
        for (clay_buffers) |*clay_buffer| {
            clay_buffer.deinit(allocator);
        }
        allocator.free(clay_buffers);
    }

    for (clay_buffers) |clay_buffer| {
        var render_buffer: RenderBuffer = .init();
        try render_buffer.setTransientBuffer(factory, clay_buffer);

        try self._commands.addCommand(.{
            .buffer = render_buffer,
            .texture = self._default_texture,
            .texture_flags = 0,
            .shader = self.common_shader,
            .sampler = try self.common_shader.getUniform("s_tex_color"),
            .state = Renderer.ui_state,
        });
    }
}

fn initClay(self: *Self, gpa: std.mem.Allocator) !void {
    const min_size = clay.minMemorySize();
    self._clay_memory = try gpa.alloc(u8, min_size);

    const bytes: f32 = @floatFromInt(min_size);
    const mb: f32 = bytes / 1024.0 / 1024.0;

    self._clay_arena = clay.createArenaWithCapacityAndMemory(
        min_size,
        @ptrCast(self._clay_memory.?.ptr),
    );

    std.log.info("Clay arena memory size: {d:.2}MB", .{mb});

    const dimensions: clay.Dimensions = .{ .width = self.size.x, .height = self.size.y };
    self._clay_context = clay.initialize(self._clay_arena, dimensions, .{
        .error_handler_function = onClayError,
    });
}

fn onClayError(error_data: clay.ErrorData) callconv(.c) void {
    std.log.warn(
        "Clay Error: {} Message: {s}",
        .{ error_data.error_type, error_data.error_text.str() },
    );
}

fn renderClayCommands(
    allocator: std.mem.Allocator,
    commands: []const clay.RenderCommand,
) ![]VertexBuffer16 {
    var buffers = try std.ArrayList(VertexBuffer16).initCapacity(allocator, commands.len);

    for (commands) |command| {
        const buffer = try renderClayCommand(allocator, command);

        if (buffer) |buf| {
            // This memory is being leaked when pushed into the buffers list.
            try buffers.append(allocator, buf);
        }
    }

    return try buffers.toOwnedSlice(allocator);
}

fn renderClayCommand(
    allocator: std.mem.Allocator,
    command: clay.RenderCommand,
) !?VertexBuffer16 {
    switch (command.command_type) {
        .rectangle => {
            const rect = rectFromBoundingBox(command.bounding_box);
            const color = hexColor(command.render_data.rectangle.background_color);
            return try render.shapes.quad(allocator, rect, color.data);
        },
        else => {
            return null;
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
