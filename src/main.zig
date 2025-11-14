const builtin = @import("builtin");
const callbacks = @import("callbacks.zig");
const core = @import("core");
const Cursor = @import("Cursor.zig");
const GUI = @import("GUI.zig");
const render = @import("render");
const stb = @import("stb");
const std = @import("std");
const version = @import("version");
const zbgfx = @import("zbgfx");
const zglfw = @import("zglfw");
const zmath = @import("zmath");

const commandline = core.commandline;

const Camera = render.Camera;
const Font = render.Font;
const MemFactory = render.MemFactory;
const Program = render.shaders.Program;
const RenderBuffer = render.RenderBuffer;
const Renderer = render.Renderer;
const Textures = render.Textures;
const Vertex = render.Vertex;
const VertexBuffer16 = render.VertexBuffer16;
const View = render.View;

const Rectf = core.math.Rectf;

var camera: Camera = undefined;
var camera_rotating = false;
var cursor: Cursor = .{};
var view_world: View = undefined;

pub fn main() !void {
    std.log.info("Welcome to LevelSketch!", .{});
    std.log.info("Version is {}.{}.{}", .{
        version.major,
        version.minor,
        version.patch,
    });

    var gpa = std.heap.GeneralPurposeAllocator(.{}).init;
    defer _ = gpa.deinit();

    const allocator = gpa.allocator();

    try commandline.init(allocator);
    defer commandline.deinit(allocator);

    try stb.init(allocator);
    defer stb.deinit();

    try zglfw.init();
    defer zglfw.terminate();

    const zglfw_version = zglfw.getVersion();
    std.log.info("glfw version is {}.{}.{}", .{
        zglfw_version.major,
        zglfw_version.minor,
        zglfw_version.patch,
    });

    zglfw.windowHint(.client_api, .no_api);
    const window = try zglfw.createWindow(960, 540, "Level Sketch", null);
    defer zglfw.destroyWindow(window);

    var bgfx_init: zbgfx.bgfx.Init = undefined;
    zbgfx.bgfx.initCtor(&bgfx_init);

    const framebuffer_size = window.getFramebufferSize();

    bgfx_init.resolution.width = @intCast(framebuffer_size[0]);
    bgfx_init.resolution.height = @intCast(framebuffer_size[1]);
    bgfx_init.platformData.ndt = null;
    bgfx_init.debug = true;

    if (builtin.target.os.tag == .windows) {
        bgfx_init.type = .Direct3D12;
    }

    var bgfx_callbacks = zbgfx.callbacks.CCallbackInterfaceT{
        .vtable = &callbacks.BGFXCallbacks.toVtbl(),
    };
    bgfx_init.callback = &bgfx_callbacks;

    bgfx_init.platformData.nwh = zglfw.getWin32Window(window);

    if (!zbgfx.bgfx.init(&bgfx_init)) {
        std.log.err("Failed to initialize bgfx.", .{});
        return;
    }
    defer zbgfx.bgfx.shutdown();

    printBGFXInfo();

    var renderer: Renderer = try .init(allocator);
    defer renderer.deinit();

    var font = Font.init(
        &renderer,
        "assets/fonts/Roboto-Regular.ttf",
        36.0,
    ) catch |err| {
        std.debug.panic("Failed to initialize font: {}", .{err});
    };
    defer font.deinit(allocator);

    var quad = try render.shapes.quad(allocator, .init(-1.0, 1.0, 1.0, -1.0), 0xFF227722);
    defer quad.deinit(allocator);

    var quad_render: RenderBuffer = .init();
    defer quad_render.deinit();

    try quad_render.setStaticBuffer(&renderer.mem_factory, &quad);

    const ui_size = 50.0;
    const ui_vertices: [4]Vertex = .{
        .init(0.0, 0.0, 0.0, 0.0, 0.0, 0xFFFFFFFF),
        .init(0.0, ui_size, 0.0, 0.0, 1.0, 0xFFFFFFFF),
        .init(ui_size, ui_size, 0.0, 1.0, 1.0, 0xFFFFFFFF),
        .init(ui_size, 0.0, 0.0, 1.0, 0.0, 0xFFFFFFFF),
    };

    const ui_indices: [6]u16 = .{ 0, 1, 2, 0, 2, 3 };

    var ui_buffer: RenderBuffer = .init();
    defer ui_buffer.deinit();

    const ui_buffer_mem_v = try renderer.mem_factory.create(@ptrCast(&ui_vertices), null, null);
    const ui_buffer_mem_i = try renderer.mem_factory.create(@ptrCast(&ui_indices), null, null);
    try ui_buffer.setStaticVertices(ui_buffer_mem_v, ui_vertices.len);
    try ui_buffer.setStaticIndices(ui_buffer_mem_i, ui_indices.len);

    camera = .{
        .position = zmath.f32x4(0.0, 0.0, -3.0, 1.0),
    };

    var shader_program: Program = .init(allocator);
    try shader_program.build(
        .{
            .varying_file_name = "common.def.sc",
            .fragment_file_name = "common_fragment.sc",
            .vertex_file_name = "common_vertex.sc",
        },
    );
    defer shader_program.deinit();

    const sampler_tex_color = try shader_program.getUniform("s_tex_color");

    zbgfx.bgfx.setDebug(zbgfx.bgfx.DebugFlags_None);
    zbgfx.bgfx.reset(
        @intCast(framebuffer_size[0]),
        @intCast(framebuffer_size[1]),
        zbgfx.bgfx.ResetFlags_Vsync,
        bgfx_init.resolution.format,
    );

    const aspect = @as(f32, @floatFromInt(framebuffer_size[0])) /
        @as(f32, @floatFromInt(framebuffer_size[1]));
    view_world = .init(0x303030FF, true);
    view_world.setPerspective(60.0, aspect);
    view_world.clear();

    var gui: GUI = try .init(&renderer);
    defer gui.deinit(allocator);

    gui.setView(
        @floatFromInt(framebuffer_size[0]),
        @floatFromInt(framebuffer_size[1]),
    );

    const state = zbgfx.bgfx.StateFlags_WriteRgb |
        zbgfx.bgfx.StateFlags_WriteA |
        zbgfx.bgfx.StateFlags_WriteZ |
        zbgfx.bgfx.StateFlags_DepthTestLess |
        zbgfx.bgfx.StateFlags_CullCw |
        zbgfx.bgfx.StateFlags_Msaa;

    // Timing
    var last_time: f64 = 0.0;

    while (!window.shouldClose() and window.getKey(.escape) != .press) {
        const current_time = zglfw.getTime();
        const delta_time: f32 = @floatCast(current_time - last_time);
        last_time = current_time;

        zglfw.pollEvents();
        renderer.update();

        updateCursor(window, &cursor);
        try updateCamera(window, cursor, delta_time);

        const size = window.getFramebufferSize();
        view_world.submitPerspective(camera, @intCast(size[0]), @intCast(size[1]));

        try renderer.textures.default.bind(sampler_tex_color.handle, 0);
        quad_render.bind(state);
        zbgfx.bgfx.submit(view_world.id, shader_program.handle.data, 255, 0);
        view_world.touch();

        try gui.draw();

        _ = zbgfx.bgfx.frame(false);
    }
}

fn printBGFXInfo() void {
    std.log.info("bgfx version is 1.{}.{}", .{ zbgfx.API_VERSION, zbgfx.REV_VERSION });

    const renderer = zbgfx.bgfx.getRendererType();
    std.log.info("Renderer type: {s}", .{zbgfx.bgfx.getRendererName(renderer)});

    const caps = zbgfx.bgfx.getCaps();
    std.log.info("Transient Max Vertex Size: {}", .{caps.*.limits.transientVbSize});
    std.log.info("Transient Max Index Size: {}", .{caps.*.limits.transientIbSize});
}

fn updateCursor(window: *zglfw.Window, target: *Cursor) void {
    const cursor_pos = window.getCursorPos();
    target.update(@intFromFloat(cursor_pos[0]), @intFromFloat(cursor_pos[1]));

    const left = zglfw.getMouseButton(window, .left);
    const middle = zglfw.getMouseButton(window, .middle);
    const right = zglfw.getMouseButton(window, .right);

    updateCursorButton(target, .left, left);
    updateCursorButton(target, .middle, middle);
    updateCursorButton(target, .right, right);
}

fn updateCursorButton(target: *Cursor, button: zglfw.MouseButton, action: zglfw.Action) void {
    const cursor_button = switch (button) {
        zglfw.MouseButton.left => Cursor.Button.left,
        zglfw.MouseButton.middle => Cursor.Button.middle,
        zglfw.MouseButton.right => Cursor.Button.right,
        else => Cursor.Button.unhandled,
    };

    const cursor_action = switch (action) {
        zglfw.Action.press => Cursor.Action.press,
        zglfw.Action.release => Cursor.Action.release,
        zglfw.Action.repeat => Cursor.Action.repeat,
    };

    target.buttons[@intFromEnum(cursor_button)] = cursor_action;
}

fn updateCamera(window: *zglfw.Window, cursor_: Cursor, delta_time: f32) !void {
    if (isPressed(window, .w)) {
        camera.move(.forward);
    } else if (isPressed(window, .s)) {
        camera.move(.backward);
    } else if (isPressed(window, .d)) {
        camera.move(.right);
    } else if (isPressed(window, .a)) {
        camera.move(.left);
    } else if (isPressed(window, .q)) {
        camera.move(.up);
    } else if (isPressed(window, .e)) {
        camera.move(.down);
    }

    camera.update(delta_time);

    if (cursor_.pressed(.right)) {
        const delta = cursor_.delta();
        if (!delta.isZero()) {
            if (!camera_rotating) {
                try toggleCursor(window, false);
                camera_rotating = true;
            }

            const yaw: f32 = @floatFromInt(delta.x);
            const pitch: f32 = @floatFromInt(delta.y);
            camera.rotate(pitch, yaw);
        }
    } else if (cursor_.released(.right)) {
        if (camera_rotating) {
            try toggleCursor(window, true);
            camera_rotating = false;
        }
    }
}

fn toggleCursor(window: *zglfw.Window, enabled: bool) !void {
    try zglfw.setInputMode(window, .cursor, if (enabled) .normal else .disabled);
    if (zglfw.rawMouseMotionSupported()) {
        try zglfw.setInputMode(window, .raw_mouse_motion, enabled);
    }
}

fn isPressed(window: *zglfw.Window, key: zglfw.Key) bool {
    const action = window.getKey(key);
    return action == .press;
}

// The code below will ensure that all referenced files will have their
// tests run.
//
// Seems like this is a temporary hack. More information found in the
// following thread:
// https://ziggit.dev/t/how-do-i-get-zig-build-to-run-all-the-tests/4434.
test "refall" {
    std.testing.refAllDecls(@This());
}
