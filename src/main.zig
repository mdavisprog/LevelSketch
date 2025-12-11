const builtin = @import("builtin");
const callbacks = @import("callbacks.zig");
const core = @import("core");
const gui = @import("gui");
const io = @import("io");
const render = @import("render");
const stb = @import("stb");
const std = @import("std");
const version = @import("version");
const zbgfx = @import("zbgfx");
const zglfw = @import("zglfw");

const commandline = core.commandline;
const Mat = core.math.Mat;
const Vec = core.math.Vec;

const Cursor = gui.Cursor;
const GUI = gui.GUI;

const Model = io.obj.Model;

const Camera = render.Camera;
const Mesh = render.Mesh;
const Renderer = render.Renderer;
const View = render.View;

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

    var renderer = try allocator.create(Renderer);
    renderer.* = try .init(allocator);
    defer {
        renderer.*.deinit();
        allocator.destroy(renderer);
    }

    renderer.*.framebuffer_size = .init(
        @floatFromInt(framebuffer_size[0]),
        @floatFromInt(framebuffer_size[1]),
    );

    const meshes = try loadCommandLineModels(renderer);
    defer {
        for (meshes) |*mesh| {
            mesh.deinit();
        }
        renderer.allocator.free(meshes);
    }

    camera = .{
        .position = .init(0.0, 0.0, -3.0, 1.0),
    };

    var shader_program = try renderer.programs.build(
        allocator,
        "common",
        .{
            .varying_file_name = "common.def.sc",
            .fragment_file_name = "common_fragment.sc",
            .vertex_file_name = "common_vertex.sc",
        },
    );

    const phong_shader = try renderer.programs.build(
        allocator,
        "phong",
        .{
            .varying_file_name = "phong.def.sc",
            .fragment_file_name = "phong_fragment.sc",
            .vertex_file_name = "phong_vertex.sc",
        },
    );
    const u_normal_mat = try phong_shader.getUniform("u_normal_mat");
    const u_view_pos = try phong_shader.getUniform("u_view_pos");

    var light: render.materials.Light = .{
        .ambient = .init(0.2, 0.2, 0.2, 1.0),
        .diffuse = .init(0.5, 0.5, 0.5, 1.0),
        .specular = .splat(1.0),
    };
    try light.bind(phong_shader);

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

    var main_gui: GUI = try .init(renderer);
    defer main_gui.deinit(allocator);

    // Timing
    var last_time: f64 = 0.0;

    var cube_yaw: f32 = 0.0;
    var cube = try render.shapes.cube(u16, renderer, .splat(0.2), 0xFFFFFFFF);
    defer cube.deinit();
    const model_transform: Mat = .identity;

    while (!window.shouldClose() and window.getKey(.escape) != .press) {
        const current_time = zglfw.getTime();
        const delta_time: f32 = @floatCast(current_time - last_time);
        last_time = current_time;

        zglfw.pollEvents();
        updateCursor(window, &cursor);
        try updateCamera(window, cursor, delta_time);

        renderer.update();
        try main_gui.update(renderer, delta_time, cursor.current.toVec2f());

        const size = window.getFramebufferSize();
        view_world.submitPerspective(camera, @intCast(size[0]), @intCast(size[1]));

        // Render the world
        u_view_pos.setArray(&camera.position.toArray());

        try renderer.textures.default.bind(sampler_tex_color.handle, 0);

        // Update light source (cube).
        cube_yaw = @mod(cube_yaw + delta_time * 20.0, 360.0);
        const cube_sin = std.math.sin(@as(f32, @floatCast(current_time)));
        light.position = Mat.initTranslation(.right)
            .rotateY(cube_yaw)
            .scale(.splat(2.0))
            .getTranslation();
        light.position.setY(cube_sin);
        const cube_transform = Mat.initTranslation(light.position);
        try light.bindPosition(phong_shader);
        _ = zbgfx.bgfx.setTransform(&cube_transform.toArray(), 1);
        cube.buffer.bind(Renderer.world_state);
        zbgfx.bgfx.submit(view_world.id, shader_program.handle.data, 0, 0);

        const normal_mat = model_transform.inverse().transpose();
        _ = zbgfx.bgfx.setTransform(&model_transform.toArray(), 1);
        u_normal_mat.setMat(normal_mat);
        for (meshes) |mesh| {
            mesh.buffer.bind(Renderer.world_state);
            try mesh.phong.bind(phong_shader);
            zbgfx.bgfx.submit(view_world.id, phong_shader.handle.data, 0, 0);
        }

        view_world.touch();

        // Render the GUI
        try main_gui.draw(renderer);

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

fn loadCommandLineModels(renderer: *Renderer) ![]Mesh {
    const allocator = renderer.allocator;

    var meshes = try std.ArrayList(Mesh).initCapacity(allocator, 0);
    defer meshes.deinit(allocator);

    const file_names = try commandline.getArgValues(allocator, "--model") orelse
        return try meshes.toOwnedSlice(allocator);
    defer allocator.free(file_names);

    for (file_names) |file_name| {
        const path = std.fs.cwd().realpathAlloc(renderer.allocator, file_name) catch |err| {
            std.log.warn("Failed load model file {s}. Error: {}", .{ file_name, err });
            continue;
        };
        defer renderer.allocator.free(path);

        var model = try io.obj.Model.loadFile(allocator, path);
        defer model.deinit(allocator);

        const mesh = try Mesh.init(renderer, model);
        try meshes.append(allocator, mesh);
    }

    return try meshes.toOwnedSlice(allocator);
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
