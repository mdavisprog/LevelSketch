const app = @import("app");
const builtin = @import("builtin");
const callbacks = @import("callbacks.zig");
const core = @import("core");
const glfw = @import("glfw.zig");
const gui = @import("gui");
const io = @import("io");
const render = @import("render");
const stb = @import("stb");
const std = @import("std");
const version = @import("version");
const world = @import("world");
const zbgfx = @import("zbgfx");
const zglfw = @import("zglfw");

const Camera = world.Camera;
const commandline = core.commandline;
const Cursor = gui.Cursor;
const GUI = gui.GUI;
const Mat = core.math.Mat;
const Meshes = render.Meshes;
const Model = io.obj.Model;
const Renderer = render.Renderer;
const Vec = core.math.Vec;
const View = render.View;
const World = world.World;

var camera_rotating = false;
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

    try glfw.init();
    defer glfw.deinit();

    var bgfx_init: zbgfx.bgfx.Init = undefined;
    zbgfx.bgfx.initCtor(&bgfx_init);

    const framebuffer_size = glfw.primary_window.framebufferSize();

    bgfx_init.resolution.width = framebuffer_size.x;
    bgfx_init.resolution.height = framebuffer_size.y;
    bgfx_init.platformData.ndt = null;
    bgfx_init.debug = true;

    if (builtin.target.os.tag == .windows) {
        bgfx_init.type = .Direct3D12;
    }

    var bgfx_callbacks = zbgfx.callbacks.CCallbackInterfaceT{
        .vtable = &callbacks.BGFXCallbacks.toVtbl(),
    };
    bgfx_init.callback = &bgfx_callbacks;
    bgfx_init.platformData.nwh = glfw.primary_window.nativeHandle();

    if (!zbgfx.bgfx.init(&bgfx_init)) {
        std.log.err("Failed to initialize bgfx.", .{});
        return;
    }
    defer zbgfx.bgfx.shutdown();

    printBGFXInfo();

    var renderer = try allocator.create(Renderer);
    renderer.* = try .init(allocator);
    defer {
        renderer.deinit();
        allocator.destroy(renderer);
    }

    renderer.framebuffer_size = framebuffer_size.to(f32);

    var shader_program = try renderer.programs.build(
        allocator,
        "common",
        .{
            .varying_file_name = "common/def.sc",
            .fragment_file_name = "common/fragment.sc",
            .vertex_file_name = "common/vertex.sc",
        },
    );

    const phong_shader = try renderer.programs.build(
        allocator,
        "phong",
        .{
            .varying_file_name = "phong/def.sc",
            .fragment_file_name = "phong/fragment.sc",
            .vertex_file_name = "phong/vertex.sc",
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
        framebuffer_size.x,
        framebuffer_size.y,
        zbgfx.bgfx.ResetFlags_Vsync,
        bgfx_init.resolution.format,
    );

    const aspect = @as(f32, @floatFromInt(framebuffer_size.x)) /
        @as(f32, @floatFromInt(framebuffer_size.y));
    view_world = .init(0x303030FF, true);
    view_world.setPerspective(60.0, aspect);
    view_world.clear();

    var main_gui: GUI = try .init(renderer);
    defer main_gui.deinit(allocator);

    // Timing
    var last_time: f64 = 0.0;

    var cube_yaw: f32 = 0.0;
    var cube_time: f32 = 0.0;
    const cube = try render.shapes.cube(u16, renderer, .splat(0.2), 0xFFFFFFFF);
    const model_transform: Mat = .identity;

    var the_world: World = try .init(allocator);
    defer the_world.deinit();
    the_world.camera.position = .init(0.0, 0.0, -3.0, 1.0);

    const meshes = try loadCommandLineModels(renderer);
    defer allocator.free(meshes);

    the_world.runSystems(.startup);

    while (!glfw.primary_window.shouldClose() and !app.State.should_exit) {
        const current_time = zglfw.getTime();
        const delta_time: f32 = @floatCast(current_time - last_time);
        last_time = current_time;

        if (the_world.getResource(world.resources.core.Frame)) |frame| {
            frame.count += 1;
            frame.times.current = current_time;
            frame.times.elapsed += current_time - last_time;
            frame.times.delta = delta_time;
        }

        glfw.update();
        try updateCamera(&the_world.camera, glfw.primary_window, delta_time);

        if (glfw.primary_window.isPressed(.escape)) {
            app.State.should_exit = true;
        }

        the_world.runSystems(.update);

        renderer.update();
        try main_gui.update(renderer, &the_world, delta_time, glfw.primary_window.cursor);

        const size = glfw.primary_window.framebufferSize();
        view_world.submitPerspective(the_world.camera.toLookAt(), @intCast(size.x), @intCast(size.y));

        // Render the world
        u_view_pos.setArray(&the_world.camera.position.toArray());

        try renderer.textures.default.bind(sampler_tex_color.handle, 0);

        // Update light source (cube).
        if (the_world.light_orbit) {
            cube_yaw = @mod(cube_yaw + delta_time * 20.0, 360.0);
            cube_time += delta_time;
            const cube_sin = std.math.sin(cube_time);
            light.position = Mat.initTranslation(.right)
                .rotateY(cube_yaw)
                .scale(.splat(2.0))
                .getTranslation();
            light.position.setY(cube_sin);
        }
        const cube_transform = Mat.initTranslation(light.position);
        try light.bindPosition(phong_shader);
        _ = zbgfx.bgfx.setTransform(&cube_transform.toArray(), 1);
        try renderer.meshes.bind(cube, null);
        zbgfx.bgfx.submit(view_world.id, shader_program.handle.data, 0, 0);

        const normal_mat = model_transform.inverse().transpose();
        _ = zbgfx.bgfx.setTransform(&model_transform.toArray(), 1);
        u_normal_mat.setMat(normal_mat);
        for (meshes) |mesh| {
            try renderer.meshes.bind(mesh, phong_shader);
            zbgfx.bgfx.submit(view_world.id, phong_shader.handle.data, 0, 0);
        }

        view_world.touch();

        // Render the GUI
        try main_gui.draw(renderer);

        _ = zbgfx.bgfx.frame(false);
    }

    the_world.runSystems(.shutdown);
}

fn printBGFXInfo() void {
    std.log.info("bgfx version is 1.{}.{}", .{ zbgfx.version.api, zbgfx.version.rev });

    const renderer = zbgfx.bgfx.getRendererType();
    std.log.info("Renderer type: {s}", .{zbgfx.bgfx.getRendererName(renderer)});

    const caps = zbgfx.bgfx.getCaps();
    std.log.info("Transient Max Vertex Size: {}", .{caps.*.limits.transientVbSize});
    std.log.info("Transient Max Index Size: {}", .{caps.*.limits.transientIbSize});
}

/// TODO: Move this logic into the camera.
/// Should accept an input object that is translated from the glfw.Window object.
fn updateCamera(camera: *Camera, window: glfw.Window, delta_time: f32) !void {
    if (window.isPressed(.w)) {
        camera.move(.forward);
    } else if (window.isPressed(.s)) {
        camera.move(.backward);
    } else if (window.isPressed(.d)) {
        camera.move(.right);
    } else if (window.isPressed(.a)) {
        camera.move(.left);
    } else if (window.isPressed(.q)) {
        camera.move(.up);
    } else if (window.isPressed(.e)) {
        camera.move(.down);
    }

    camera.update(delta_time);

    if (window.cursor.pressed(.right)) {
        const delta = window.cursor.delta();
        if (!delta.isZero()) {
            if (!camera_rotating) {
                try window.toggleCursor(false);
                camera_rotating = true;
            }

            const yaw: f32 = @floatFromInt(delta.x);
            const pitch: f32 = @floatFromInt(delta.y);
            camera.rotate(pitch, yaw);
        }
    } else if (window.cursor.released(.right)) {
        if (camera_rotating) {
            try window.toggleCursor(true);
            camera_rotating = false;
        }
    }
}

fn loadCommandLineModels(renderer: *Renderer) ![]Meshes.Id {
    const allocator = renderer.allocator;

    var meshes = try std.ArrayList(Meshes.Id).initCapacity(allocator, 0);
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

        const mesh = try renderer.loadMeshFromModel(model);
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
