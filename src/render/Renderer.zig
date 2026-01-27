const core = @import("core");
const ecs = @import("ecs/root.zig");
const io = @import("io");
const render = @import("root.zig");
const std = @import("std");
const world = @import("world");
const zbgfx = @import("zbgfx");

const Fonts = render.Fonts;
const MemFactory = render.MemFactory;
const Meshes = render.Meshes;
const Model = io.obj.Model;
const Program = render.shaders.Program;
const Programs = render.shaders.Programs;
const RenderBuffer = render.RenderBuffer;
const SystemParam = world.Systems.SystemParam;
const Textures = render.Textures;
const Vec = core.math.Vec;
const Vec2f = core.math.Vec2f;
const Vec2u = core.math.Vec2u;
const VertexBuffer16 = render.VertexBuffer16;
const VertexBuffer32 = render.VertexBuffer32;
const VertexBufferUploads16 = render.VertexBufferUploads16;
const VertexBufferUploads32 = render.VertexBufferUploads32;
const View = render.View;
const World = world.World;

const Self = @This();

pub const world_state = zbgfx.bgfx.StateFlags_WriteRgb |
    zbgfx.bgfx.StateFlags_WriteA |
    zbgfx.bgfx.StateFlags_WriteZ |
    zbgfx.bgfx.StateFlags_DepthTestLess |
    zbgfx.bgfx.StateFlags_CullCw |
    zbgfx.bgfx.StateFlags_Msaa;

pub const ui_state = zbgfx.bgfx.StateFlags_WriteRgb |
    zbgfx.bgfx.StateFlags_WriteA |
    zbgfx.bgfx.StateFlags_Msaa |
    render.stateFlagsBlend(
        zbgfx.bgfx.StateFlags_BlendSrcAlpha,
        zbgfx.bgfx.StateFlags_BlendInvSrcAlpha,
    );

mem_factory: MemFactory,
textures: Textures,
programs: Programs,
fonts: *Fonts,
meshes: Meshes,
framebuffer_size: Vec2f = .zero,
allocator: std.mem.Allocator,
view_world: View,
_uploads16: VertexBufferUploads16,
_uploads32: VertexBufferUploads32,

pub fn init(allocator: std.mem.Allocator) !Self {
    var mem_factory = try MemFactory.init(allocator);
    const textures = try Textures.init(&mem_factory);
    var programs: Programs = .init();
    const fonts: *Fonts = try .init(allocator);

    _ = try programs.buildWithName(
        allocator,
        "common",
        .{
            .varying_file_name = "common/def.sc",
            .fragment_file_name = "common/fragment.sc",
            .vertex_file_name = "common/vertex.sc",
        },
    );

    _ = try programs.buildWithName(
        allocator,
        "phong",
        .{
            .varying_file_name = "phong/def.sc",
            .fragment_file_name = "phong/fragment.sc",
            .vertex_file_name = "phong/vertex.sc",
        },
    );

    const view: View = .init(0x303030FF, true);
    // This needs to be a separate call. This notifies bgfx that this view will perform the
    // clear operation. Other views should not be performing any clears.
    view.clear();

    return .{
        .mem_factory = mem_factory,
        .textures = textures,
        .programs = programs,
        .fonts = fonts,
        .meshes = .init(),
        .allocator = allocator,
        .view_world = view,
        ._uploads16 = try .init(allocator),
        ._uploads32 = try .init(allocator),
    };
}

pub fn deinit(self: *Self) void {
    self.meshes.deinit(self.allocator);
    self.fonts.deinit(self.allocator);
    self.allocator.destroy(self.fonts);

    self.programs.deinit(self.allocator);
    self.textures.deinit(self.allocator);
    self.mem_factory.deinit();

    self._uploads16.deinit(self.allocator);
    self._uploads32.deinit(self.allocator);
}

pub fn initECS(self: *Self, _world: *World) !void {
    try _world.registerResource(ecs.resources.Render, .{
        .renderer = self,
    });
    try _world.registerResource(ecs.resources.Lights, .{});

    try _world.registerComponents(&.{
        ecs.components.Mesh,
        ecs.components.Phong,
        ecs.components.Light,
        ecs.components.Color,
    });

    _ = try _world.registerSystem(&.{}, .update, updateSystem);
    // TODO: Need a way to render all meshes and their materials in a single system.
    _ = try _world.registerSystem(
        &.{
            world.components.core.Transform,
            ecs.components.Mesh,
            ecs.components.Phong,
        },
        .render,
        renderPhong,
    );
    _ = try _world.registerSystem(
        &.{
            world.components.core.Transform,
            ecs.components.Mesh,
            ecs.components.Color,
        },
        .render,
        renderColor,
    );
    _ = try _world.registerSystem(&.{}, .shutdown, shutdownSystem);
}

pub fn updateView(self: *Self, size: Vec2u) void {
    self.framebuffer_size = size.to(f32);
    const aspect = self.framebuffer_size.x / self.framebuffer_size.y;
    self.view_world.setPerspective(60.0, aspect);
}

pub fn uploadVertexBuffer(self: *Self, buffer: anytype) !RenderBuffer {
    const BufferType = @TypeOf(buffer);
    if (BufferType == VertexBuffer16) {
        return try self._uploads16.addUpload(&self.mem_factory, buffer);
    } else if (BufferType == VertexBuffer32) {
        return try self._uploads32.addUpload(&self.mem_factory, buffer);
    } else {
        @compileError(std.fmt.comptimePrint(
            "Invalid buffer type given: {s}.",
            .{@typeName(BufferType)},
        ));
    }
}

pub fn loadMeshFromModel(self: *Self, model: Model) !Meshes.Mesh.Handle {
    return self.meshes.loadFromModel(self, model);
}

pub fn loadMeshFromBuffer(self: *Self, buffer: anytype) !Meshes.Mesh.Handle {
    return self.meshes.loadFromBuffer(self, buffer);
}

fn shutdownSystem(param: SystemParam) !void {
    const lights = param.world.getResource(ecs.resources.Lights) orelse unreachable;
    lights.entities.deinit(param.world._allocator);
}

fn updateSystem(param: SystemParam) !void {
    const resource = param.world.getResource(ecs.resources.Render) orelse unreachable;
    resource.renderer.mem_factory.update();
    resource.renderer._uploads16.update(resource.renderer.allocator);
    resource.renderer._uploads32.update(resource.renderer.allocator);
}

fn renderPhong(param: SystemParam) !void {
    const _render = param.world.getResource(ecs.resources.Render) orelse unreachable;
    const renderer = _render.renderer;
    // TODO: For now, all light entities are added to a resource. Should allow this system to
    // query for light entities.
    const lights_resource = param.world.getResource(ecs.resources.Lights) orelse unreachable;
    const phong = renderer.programs.getByName("phong") orelse unreachable;

    var lights = lights_resource.entities.iterator();
    while (lights.next()) |light| {
        const light_transform = param.world.getComponent(world.components.core.Transform, light.*) orelse continue;
        const light_component = param.world.getComponent(ecs.components.Light, light.*) orelse continue;

        try phong.setUniform("u_light_position", light_transform.translation);
        try phong.setUniform("u_light_ambient", light_component.ambient);
        try phong.setUniform("u_light_diffuse", light_component.diffuse);
        try phong.setUniform("u_light_specular", light_component.specular);

        var entities = param.entities.iterator();
        while (entities.next()) |entity| {
            const transform = param.world.getComponent(world.components.core.Transform, entity.*) orelse continue;
            const mesh_component = param.world.getComponent(ecs.components.Mesh, entity.*) orelse continue;
            const mesh = renderer.meshes.get(mesh_component.handle) orelse continue;
            const material = param.world.getComponent(ecs.components.Phong, entity.*) orelse continue;

            mesh.buffer.bind(world_state);
            try phong.setTexture("s_diffuse", material.diffuse, 0);
            try phong.setTexture("s_specular", material.specular, 1);
            try phong.setUniform("u_diffuse", material.diffuse_color);
            try phong.setUniform("u_specular_shininess", Vec.init(
                material.specular_color.x(),
                material.specular_color.y(),
                material.specular_color.z(),
                material.shininess,
            ));

            const model_mat = transform.toMatrix();
            const normal_mat = model_mat.inverse().transpose();

            _ = zbgfx.bgfx.setTransform(&model_mat.toArray(), 1);
            try phong.setUniform("u_normal_mat", normal_mat);

            zbgfx.bgfx.submit(renderer.view_world.id, phong.bgfx_handle, 0, 0);
            zbgfx.bgfx.discard(zbgfx.bgfx.DiscardFlags_All);
        }
    }
}

fn renderColor(param: SystemParam) !void {
    const _render = param.world.getResource(ecs.resources.Render) orelse unreachable;
    const renderer = _render.renderer;
    const common = renderer.programs.getByName("common") orelse unreachable;
    const sampler = try common.getUniform("s_tex_color");

    try renderer.textures.default.bind(sampler.handle, 0);

    var entities = param.entities.iterator();
    while (entities.next()) |entity| {
        const transform = param.world.getComponent(world.components.core.Transform, entity.*) orelse continue;
        const mesh_component = param.world.getComponent(ecs.components.Mesh, entity.*) orelse continue;
        const mesh = renderer.meshes.get(mesh_component.handle) orelse continue;
        const material = param.world.getComponent(ecs.components.Color, entity.*) orelse continue;

        mesh.buffer.bind(world_state);
        _ = zbgfx.bgfx.setTransform(&transform.toMatrix().toArray(), 1);
        try common.setUniform("u_color", material.tint);
        zbgfx.bgfx.submit(renderer.view_world.id, common.bgfx_handle, 0, 0);
        zbgfx.bgfx.discard(zbgfx.bgfx.DiscardFlags_All);
    }
}
