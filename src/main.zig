const builtin = @import("builtin");
const callbacks = @import("callbacks.zig");
const core = @import("core");
const Cursor = @import("Cursor.zig");
const io = @import("io");
const render = @import("render");
const stb = @import("stb");
const std = @import("std");
const version = @import("version");
const zbgfx = @import("zbgfx");
const zglfw = @import("zglfw");
const zmath = @import("zmath");

const commandline = core.commandline;

const Camera = render.Camera;
const Vertex = render.Vertex;
const View = render.View;

var camera: Camera = undefined;
var camera_rotating = false;
var cursor: Cursor = .{};
var view_world: View = undefined;
var view_ui: View = undefined;

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

    const info_tex_path = try io.exeRelativePath(allocator, &.{"assets/textures/info.png"});
    defer allocator.free(info_tex_path);

    const info_tex = try io.getContents(allocator, info_tex_path);
    defer allocator.free(info_tex);

    const info_tex_data = try stb.image.load_from_memory(info_tex);
    defer stb.image.free(info_tex_data);

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

    const vertices: [3]Vertex = .{
        .init(-0.5, -0.5, 0.5, 0xFF0000FF),
        .init(0.5, -0.5, 0.5, 0xFF00FF00),
        .init(0.0, 0.5, 0.5, 0xFFFF0000),
    };

    const indices: [3]u16 = .{ 0, 1, 2 };

    const ui_vertices: [4]Vertex = .{
        .init(0.0, 0.0, 0.0, 0xFFFF0000),
        .init(0.0, 50.0, 0.0, 0xFFFF0000),
        .init(50.0, 50.0, 0.0, 0xFFFF0000),
        .init(50.0, 0.0, 0.0, 0xFFFF0000),
    };

    const ui_indices: [6]u16 = .{ 0, 1, 2, 0, 2, 3 };

    const layout = Vertex.Layout.init();
    const vertex_buffer = zbgfx.bgfx.createVertexBuffer(
        zbgfx.bgfx.makeRef(&vertices, vertices.len * @sizeOf(Vertex)),
        &layout.data,
        zbgfx.bgfx.BufferFlags_None,
    );
    defer zbgfx.bgfx.destroyVertexBuffer(vertex_buffer);

    const index_buffer = zbgfx.bgfx.createIndexBuffer(
        zbgfx.bgfx.makeRef(&indices, vertices.len * @sizeOf(u16)),
        zbgfx.bgfx.BufferFlags_None,
    );
    defer zbgfx.bgfx.destroyIndexBuffer(index_buffer);

    const ui_vertex_buffer = zbgfx.bgfx.createVertexBuffer(
        zbgfx.bgfx.makeRef(&ui_vertices, ui_vertices.len * @sizeOf(Vertex)),
        &layout.data,
        zbgfx.bgfx.BufferFlags_None,
    );
    defer zbgfx.bgfx.destroyVertexBuffer(ui_vertex_buffer);

    const ui_index_buffer = zbgfx.bgfx.createIndexBuffer(
        zbgfx.bgfx.makeRef(&ui_indices, ui_indices.len * @sizeOf(u16)),
        zbgfx.bgfx.BufferFlags_None,
    );
    defer zbgfx.bgfx.destroyIndexBuffer(ui_index_buffer);

    camera = .{
        .position = zmath.f32x4(0.0, 0.0, -3.0, 1.0),
    };

    const mem = zbgfx.bgfx.makeRef(
        info_tex_data.data,
        @intCast(info_tex_data.size()),
    );
    const info_tex_handle = zbgfx.bgfx.createTexture2D(
        info_tex_data.width,
        info_tex_data.height,
        false,
        1,
        zbgfx.bgfx.TextureFormat.RGBA8,
        zbgfx.bgfx.TextureFlags_None,
        mem,
    );
    defer zbgfx.bgfx.destroyTexture(info_tex_handle);

    var shader_program = render.shaders.Program{};
    _ = try shader_program.build(
        allocator,
        .{
            .varying_file_name = "common.def.sc",
            .fragment_file_name = "common_fragment.sc",
            .vertex_file_name = "common_vertex.sc",
        },
    );
    defer shader_program.clean();

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

    view_ui = .init(0x000000FF, false);
    view_ui.setOrthographic(
        @floatFromInt(framebuffer_size[0]),
        @floatFromInt(framebuffer_size[1]),
    );
    view_ui.setMode(.Sequential);

    const state = zbgfx.bgfx.StateFlags_WriteRgb |
        zbgfx.bgfx.StateFlags_WriteA |
        zbgfx.bgfx.StateFlags_WriteZ |
        zbgfx.bgfx.StateFlags_DepthTestLess |
        zbgfx.bgfx.StateFlags_CullCw |
        zbgfx.bgfx.StateFlags_Msaa;

    const ui_state = zbgfx.bgfx.StateFlags_WriteRgb |
        zbgfx.bgfx.StateFlags_WriteA |
        zbgfx.bgfx.StateFlags_Msaa;

    // Timing
    var last_time: f64 = 0.0;

    while (!window.shouldClose() and window.getKey(.escape) != .press) {
        const current_time = zglfw.getTime();
        const delta_time: f32 = @floatCast(current_time - last_time);
        last_time = current_time;

        zglfw.pollEvents();

        updateCursor(window, &cursor);
        try updateCamera(window, cursor, delta_time);

        const size = window.getFramebufferSize();
        view_world.submitPerspective(camera, @intCast(size[0]), @intCast(size[1]));

        zbgfx.bgfx.setVertexBuffer(0, vertex_buffer, 0, vertices.len);
        zbgfx.bgfx.setIndexBuffer(index_buffer, 0, indices.len);
        zbgfx.bgfx.setState(state, 0);
        zbgfx.bgfx.submit(view_world.id, shader_program.handle, 0, 255);
        view_world.touch();

        view_ui.submitOrthographic(@intCast(size[0]), @intCast(size[1]));

        zbgfx.bgfx.setVertexBuffer(0, ui_vertex_buffer, 0, ui_vertices.len);
        zbgfx.bgfx.setIndexBuffer(ui_index_buffer, 0, ui_indices.len);
        zbgfx.bgfx.setState(ui_state, 0);
        zbgfx.bgfx.submit(view_ui.id, shader_program.handle, 0, 255);
        view_ui.touch();

        _ = zbgfx.bgfx.frame(false);
    }
}

fn printBGFXInfo() void {
    std.log.info("bgfx version is 1.{}.{}", .{ zbgfx.API_VERSION, zbgfx.REV_VERSION });

    const renderer = zbgfx.bgfx.getRendererType();
    std.log.info("Renderer type: {s}", .{zbgfx.bgfx.getRendererName(renderer)});
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
