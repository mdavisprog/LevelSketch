const builtin = @import("builtin");
const callbacks = @import("callbacks.zig");
const core = @import("core");
const gui = @import("gui");
const io = @import("io");
const platform = @import("platform");
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

    var the_world: World = try .init(allocator);
    defer the_world.deinit();
    the_world.camera.position = .init(0.0, 0.0, -3.0, 1.0);

    try platform.init(&the_world);
    const platform_resource = the_world.getResource(platform.resources.Platform).?;
    const mouse = the_world.getResource(platform.input.resources.Mouse).?;

    var bgfx_init: zbgfx.bgfx.Init = undefined;
    zbgfx.bgfx.initCtor(&bgfx_init);

    const framebuffer_size = platform_resource.primary_window.framebufferSize();

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
    bgfx_init.platformData.nwh = platform_resource.primary_window.nativeHandle();

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
    const common = renderer.programs.getByName("common").?;
    const phong = renderer.programs.getByName("phong").?;
    const u_view_pos = try phong.getUniform("u_view_pos");

    var light: render.materials.Light = .{
        .ambient = .init(0.2, 0.2, 0.2, 1.0),
        .diffuse = .init(0.5, 0.5, 0.5, 1.0),
        .specular = .splat(1.0),
    };
    try light.bind(phong);

    const sampler_tex_color = try common.getUniform("s_tex_color");

    zbgfx.bgfx.setDebug(zbgfx.bgfx.DebugFlags_None);
    zbgfx.bgfx.reset(
        framebuffer_size.x,
        framebuffer_size.y,
        zbgfx.bgfx.ResetFlags_Vsync,
        bgfx_init.resolution.format,
    );

    var main_gui: GUI = try .init(renderer);
    defer main_gui.deinit(allocator);

    // Timing
    var last_time: f64 = 0.0;

    var cube_yaw: f32 = 0.0;
    var cube_time: f32 = 0.0;
    const cube = try render.shapes.cube(u16, renderer, .splat(0.2), 0xFFFFFFFF);

    _ = try the_world.registerSystem(&.{}, .update, updateCamera);
    try renderer.initECS(&the_world);

    try loadCommandLineModels(renderer, &the_world);

    the_world.runSystems(.startup);

    while (!platform_resource.primary_window.shouldClose()) {
        const current_time = zglfw.getTime();
        const delta_time: f32 = @floatCast(current_time - last_time);
        last_time = current_time;

        if (the_world.getResource(world.resources.core.Frame)) |frame| {
            frame.count += 1;
            frame.times.current = current_time;
            frame.times.elapsed += current_time - last_time;
            frame.times.delta = delta_time;
        }

        //try updateCamera(&the_world.camera, platform_resource.primary_window, delta_time);

        the_world.runSystems(.update);

        try main_gui.update(renderer, &the_world, delta_time, mouse.state);

        const size = platform_resource.primary_window.framebufferSize();
        renderer.updateView(size);
        renderer.view_world.submitPerspective(the_world.camera.toLookAt(), @intCast(size.x), @intCast(size.y));

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
        try light.bindPosition(phong);
        _ = zbgfx.bgfx.setTransform(&cube_transform.toArray(), 1);
        try renderer.meshes.bind(cube, null);
        zbgfx.bgfx.submit(renderer.view_world.id, common.bgfx_handle, 0, 0);

        the_world.runSystems(.render);
        renderer.view_world.touch();

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
fn updateCamera(param: world.Systems.SystemParam) void {
    const frame = param.world.getResource(world.resources.core.Frame) orelse return;
    const _platform = param.world.getResource(platform.resources.Platform) orelse return;
    const keyboard = param.world.getResource(platform.input.resources.Keyboard) orelse return;
    const mouse = param.world.getResource(platform.input.resources.Mouse) orelse return;
    const camera: *Camera = &param.world.camera;

    if (keyboard.state.isPressed(.w)) {
        camera.move(.forward);
    } else if (keyboard.state.isPressed(.s)) {
        camera.move(.backward);
    } else if (keyboard.state.isPressed(.d)) {
        camera.move(.right);
    } else if (keyboard.state.isPressed(.a)) {
        camera.move(.left);
    } else if (keyboard.state.isPressed(.q)) {
        camera.move(.up);
    } else if (keyboard.state.isPressed(.e)) {
        camera.move(.down);
    }

    camera.update(frame.times.delta);

    if (mouse.state.pressed(.right)) {
        const delta = mouse.state.delta();
        if (!delta.isZero()) {
            if (!camera_rotating) {
                _platform.primary_window.toggleCursor(false) catch unreachable;
                camera_rotating = true;
            }

            const yaw: f32 = delta.x;
            const pitch: f32 = delta.y;
            camera.rotate(pitch, yaw);
        }
    } else if (mouse.state.released(.right)) {
        if (camera_rotating) {
            _platform.primary_window.toggleCursor(true) catch unreachable;
            camera_rotating = false;
        }
    }
}

fn loadCommandLineModels(renderer: *Renderer, _world: *World) !void {
    const allocator = renderer.allocator;

    const file_names = try commandline.getArgValues(allocator, "--model") orelse return;
    defer allocator.free(file_names);

    for (file_names) |file_name| {
        const path = std.fs.cwd().realpathAlloc(renderer.allocator, file_name) catch |err| {
            std.log.warn("Failed load model file {s}. Error: {}", .{ file_name, err });
            continue;
        };
        defer allocator.free(path);

        var model = try io.obj.Model.loadFile(allocator, path);
        defer model.deinit(allocator);

        const mesh = try renderer.loadMeshFromModel(model);

        const entity = _world.createEntity();
        try _world.insertComponent(world.components.core.Transform, entity, .{});
        try _world.insertComponent(render.ecs.components.Mesh, entity, .{
            .handle = mesh,
        });
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
