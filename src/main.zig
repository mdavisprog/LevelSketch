const builtin = @import("builtin");
const callbacks = @import("callbacks.zig");
const core = @import("core");
const Cursor = @import("Cursor.zig");
const render = @import("render");
const std = @import("std");
const version = @import("version");
const zbgfx = @import("zbgfx");
const zglfw = @import("zglfw");
const zmath = @import("zmath");

const commandline = core.commandline;

const Camera = render.Camera;
const Vertex = render.Vertex;

var camera: Camera = undefined;
var cursor: Cursor = .{};

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
        bgfx_init.type = zbgfx.bgfx.RendererType.Direct3D12;
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

    camera = .{
        .position = zmath.f32x4(0.0, 0.0, -3.0, 1.0),
    };

    const aspect = @as(f32, @floatFromInt(framebuffer_size[0])) / @as(f32, @floatFromInt(framebuffer_size[1]));
    const projection = zmath.perspectiveFovLh(
        std.math.degreesToRadians(60.0),
        aspect,
        0.1,
        100.0,
    );

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

    zbgfx.bgfx.setViewClear(
        0,
        zbgfx.bgfx.ClearFlags_Color | zbgfx.bgfx.ClearFlags_Depth,
        0x303030FF,
        1.0,
        0,
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

        updateCursor(window, &cursor);
        updateCamera(window, cursor, delta_time);

        const view = camera.toLookAt();
        const size = window.getFramebufferSize();
        zbgfx.bgfx.setViewTransform(0, &zmath.matToArr(view), &zmath.matToArr(projection));
        zbgfx.bgfx.setViewRect(0, 0, 0, @intCast(size[0]), @intCast(size[1]));

        zbgfx.bgfx.setVertexBuffer(0, vertex_buffer, 0, vertices.len);
        zbgfx.bgfx.setIndexBuffer(index_buffer, 0, indices.len);
        zbgfx.bgfx.setState(state, 0);
        zbgfx.bgfx.submit(0, shader_program.handle, 0, 255);

        zbgfx.bgfx.touch(0);
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

    const left = zglfw.getMouseButton(window, zglfw.MouseButton.left);
    const middle = zglfw.getMouseButton(window, zglfw.MouseButton.middle);
    const right = zglfw.getMouseButton(window, zglfw.MouseButton.right);

    updateCursorButton(target, zglfw.MouseButton.left, left);
    updateCursorButton(target, zglfw.MouseButton.middle, middle);
    updateCursorButton(target, zglfw.MouseButton.right, right);
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

fn updateCamera(window: *zglfw.Window, cursor_: Cursor, delta_time: f32) void {
    const forward = window.getKey(zglfw.Key.w);
    const backward = window.getKey(zglfw.Key.s);
    const right = window.getKey(zglfw.Key.d);
    const left = window.getKey(zglfw.Key.a);

    if (forward == zglfw.Action.press) {
        camera.moveForward();
    }

    if (backward == zglfw.Action.press) {
        camera.moveBackward();
    }

    if (right == zglfw.Action.press) {
        camera.moveRight();
    }

    if (left == zglfw.Action.press) {
        camera.moveLeft();
    }

    camera.update(delta_time);

    if (cursor_.pressed(Cursor.Button.right)) {
        const delta = cursor_.delta();
        if (!delta.isZero()) {
            const yaw: f32 = @floatFromInt(delta.x);
            const pitch: f32 = @floatFromInt(delta.y);
            camera.rotate(pitch, yaw);
        }
    }
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
