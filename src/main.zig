const builtin = @import("builtin");
const callbacks = @import("callbacks.zig");
const core = @import("core");
const editor = @import("editor");
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

const commandline = core.commandline;
const Editor = editor.Editor;
const GUI = gui.GUI;
const Mat = core.math.Mat;
const Model = io.obj.Model;
const Renderer = render.Renderer;
const World = world.World;

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

    try platform.init(&the_world);
    const platform_resource = the_world.getResource(platform.ecs.resources.Platform).?;
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

    zbgfx.bgfx.setDebug(zbgfx.bgfx.DebugFlags_None);
    zbgfx.bgfx.reset(
        framebuffer_size.x,
        framebuffer_size.y,
        zbgfx.bgfx.ResetFlags_Vsync,
        bgfx_init.resolution.format,
    );

    var main_gui: GUI = try .init(renderer);
    defer main_gui.deinit(allocator);

    try renderer.initECS(&the_world);

    var _editor: Editor = try .init(&the_world);
    defer _editor.deinit();

    try loadCommandLineModels(renderer, &the_world);

    try _editor.addLight(renderer);
    the_world.runSystems(.startup);

    while (!platform_resource.primary_window.shouldClose()) {
        the_world.runSystems(.preupdate);
        the_world.runSystems(.update);

        try main_gui.update(&the_world, mouse.state);

        the_world.runSystems(.render);

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

// TODO: Move to 'editor' module.
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

        if (model.materials.items.len > 0) {
            const material = model.materials.items[0];
            const phong: render.ecs.components.Phong = .{
                .diffuse = try loadTexture(renderer, material.diffuse_texture),
                .diffuse_color = material.diffuse,
                .specular = try loadTexture(renderer, material.specular_texture),
                .specular_color = material.specular,
                .shininess = material.specular_exponent,
            };
            try _world.insertComponent(render.ecs.components.Phong, entity, phong);
        } else {
            try _world.insertComponent(render.ecs.components.Color, entity, .{});
        }
    }
}

fn loadTexture(renderer: *Renderer, path: ?[]const u8) !render.Texture {
    const path_ = path orelse return .{};

    return renderer.textures.loadImageAbsolute(
        &renderer.mem_factory,
        path_,
    ) catch |err| {
        std.log.warn(
            "Failed to load texture '{s}'' from model. Error: {}",
            .{ path_, err },
        );

        return renderer.textures.default;
    };
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
