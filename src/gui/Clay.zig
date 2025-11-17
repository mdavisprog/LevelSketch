const clay = @import("clay");
const core = @import("core");
const render = @import("render");
const std = @import("std");

const HexColor = core.math.HexColor;
const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;

const Commands = render.Commands;
const RenderBuffer = render.RenderBuffer;
const Renderer = render.Renderer;
const VertexBuffer16 = render.VertexBuffer16;

const Self = @This();

_memory: ?[]const u8 = null,
_arena: clay.Arena = .{},
_context: ?*clay.Context = null,

pub fn init(gpa: std.mem.Allocator) !Self {
    const min_size = clay.minMemorySize();
    const memory = try gpa.alloc(u8, min_size);

    const bytes: f32 = @floatFromInt(min_size);
    const mb: f32 = bytes / 1024.0 / 1024.0;

    const arena = clay.createArenaWithCapacityAndMemory(
        min_size,
        @ptrCast(memory.ptr),
    );

    std.log.info("Clay arena memory size: {d:.2}MB", .{mb});

    const dimensions: clay.Dimensions = .{ .width = 0, .height = 0 };
    const context = clay.initialize(arena, dimensions, .{
        .error_handler_function = onError,
    });

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

pub fn build(renderer: *Renderer, view_size: Vec2f, commands: *Commands) !void {
    const common_shader = try renderer.programs.get("common");
    const default_texture = renderer.textures.default;
    const sampler = try common_shader.getUniform("s_tex_color");

    clay.setLayoutDimensions(.init(
        view_size.x,
        view_size.y,
    ));

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

    const clay_buffers = try renderCommands(renderer._gpa, render_commands.slice());
    defer {
        for (clay_buffers) |*clay_buffer| {
            clay_buffer.deinit(renderer._gpa);
        }
        renderer._gpa.free(clay_buffers);
    }

    for (clay_buffers) |clay_buffer| {
        var render_buffer: RenderBuffer = .init();
        try render_buffer.setTransientBuffer(&renderer.mem_factory, clay_buffer);

        try commands.addCommand(.{
            .buffer = render_buffer,
            .texture = default_texture,
            .texture_flags = 0,
            .shader = common_shader,
            .sampler = sampler,
            .state = Renderer.ui_state,
        });
    }
}

fn onError(error_data: clay.ErrorData) callconv(.c) void {
    std.log.warn(
        "Clay Error: {} Message: {s}",
        .{ error_data.error_type, error_data.error_text.str() },
    );
}

fn renderCommands(
    gpa: std.mem.Allocator,
    commands: []const clay.RenderCommand,
) ![]VertexBuffer16 {
    var buffers = try std.ArrayList(VertexBuffer16).initCapacity(gpa, commands.len);

    for (commands) |command| {
        const buffer = try renderCommand(gpa, command);

        if (buffer) |buf| {
            try buffers.append(gpa, buf);
        }
    }

    return try buffers.toOwnedSlice(gpa);
}

fn renderCommand(
    gpa: std.mem.Allocator,
    command: clay.RenderCommand,
) !?VertexBuffer16 {
    switch (command.command_type) {
        .rectangle => {
            const rect = rectFromBoundingBox(command.bounding_box);
            const color = hexColor(command.render_data.rectangle.background_color);
            return try render.shapes.quad(gpa, rect, color.data);
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
