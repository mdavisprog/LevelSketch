const core = @import("core");
const io = @import("io");
const editor = @import("root.zig");
const render = @import("render");
const std = @import("std");
const world = @import("world");

const Camera = editor.components.Camera;
const Material = io.obj.Material;
const Mesh = render.ecs.components.Mesh;
const Model = io.obj.Model;
const Phong = render.ecs.components.Phong;
const Query = world.Query;
const Renderer = render.Renderer;
const SystemParam = world.SystemParam;
const Texture = render.Texture;
const Transform = world.components.core.Transform;
const Vec = core.math.Vec;

pub const ResetCamera = struct {};

pub fn onResetCamera(
    _: ResetCamera,
    cameras: Query(&.{ Camera, Transform }),
    param: SystemParam,
) !void {
    var entities = cameras.getEntities();
    while (entities.next()) |entity| {
        const transform = param.getComponent(Transform, entity.*) orelse continue;
        transform.translation = Camera.default_pos;
        transform.rotation = Vec.forward.toRotation();
    }
}

/// Event to notify the editor that some assets need to be loaded.
pub const LoadAssets = struct {
    paths: []const []const u8 = undefined,
};

pub fn onLoadAssets(assets: LoadAssets, param: SystemParam) !void {
    const render_resource = param.world.getResource(render.ecs.resources.Render) orelse unreachable;
    const renderer = render_resource.renderer;

    for (assets.paths) |path| {
        const extension = std.fs.path.extension(path);
        if (!std.mem.eql(u8, extension, ".obj")) {
            std.log.warn("Dropped file '{s}' is not a valid 'obj' file.", .{path});
            continue;
        }

        var model = io.obj.Model.loadFile(param.allocator, path) catch |err| {
            std.debug.panic("Failed to load model '{s}'. Error: {}", .{ path, err });
        };
        defer model.deinit(param.allocator);

        const mesh = try renderer.loadMeshFromModel(model);
        const material = if (model.materials.items.len > 0)
            model.materials.items[0]
        else
            Material{};

        _ = try param.world.createEntityWith(.{
            Transform{},
            Mesh{
                .handle = mesh,
            },
            try toPhong(renderer, material),
        });
    }
}

fn toPhong(renderer: *Renderer, material: Material) !Phong {
    return .{
        .diffuse = try loadTexture(renderer, material.diffuse_texture),
        .diffuse_color = material.diffuse,
        .specular = try loadTexture(renderer, material.specular_texture),
        .specular_color = material.specular,
        .shininess = material.specular_exponent,
    };
}

fn loadTexture(renderer: *Renderer, path: ?[]const u8) !Texture {
    const _path = path orelse return renderer.textures.default;
    return try renderer.loadTextureOrDefault(_path);
}
