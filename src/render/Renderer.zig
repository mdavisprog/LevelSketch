const core = @import("core");
const io = @import("io");
const render = @import("root.zig");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Fonts = render.Fonts;
const Programs = render.shaders.Programs;
const MemFactory = render.MemFactory;
const Meshes = render.Meshes;
const Model = io.obj.Model;
const RenderBuffer = render.RenderBuffer;
const Textures = render.Textures;
const Vec2f = core.math.Vec2f;
const VertexBuffer16 = render.VertexBuffer16;
const VertexBuffer32 = render.VertexBuffer32;
const VertexBufferUploads16 = render.VertexBufferUploads16;
const VertexBufferUploads32 = render.VertexBufferUploads32;

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
_uploads16: VertexBufferUploads16,
_uploads32: VertexBufferUploads32,

pub fn init(allocator: std.mem.Allocator) !Self {
    var mem_factory = try MemFactory.init(allocator);
    const textures = try Textures.init(&mem_factory);
    const programs: Programs = .init(allocator);
    const fonts: *Fonts = try .init(allocator);
    return .{
        .mem_factory = mem_factory,
        .textures = textures,
        .programs = programs,
        .fonts = fonts,
        .meshes = .init(),
        .allocator = allocator,
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

pub fn update(self: *Self) void {
    self.mem_factory.update();
    self._uploads16.update(self.allocator);
    self._uploads32.update(self.allocator);
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
