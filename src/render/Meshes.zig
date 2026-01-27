const core = @import("core");
const io = @import("io");
const render = @import("root.zig");
const std = @import("std");

const Model = io.obj.Model;
const Phong = render.materials.Phong;
const Program = render.shaders.Program;
const RenderBuffer = render.RenderBuffer;
const Renderer = render.Renderer;
const Texture = render.Texture;
const Vertex = render.Vertex;
const VertexBuffer32 = render.VertexBuffer32;

/// Manages all mesh resources.
const Self = @This();

pub const Error = error{
    InvalidModel,
    MeshNotFound,
};

/// Holds the handles and material parameters for a single mesh.
pub const Mesh = struct {
    pub const Handle = core.Handle(Mesh);

    buffer: RenderBuffer = .{},
};

pub const MeshMap = std.AutoHashMapUnmanaged(Mesh.Handle, Mesh);
const VisitMap = std.AutoHashMap(Model.Face.Element, usize);

_map: MeshMap = .empty,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    var it = self._map.valueIterator();
    while (it.next()) |mesh| {
        mesh.buffer.deinit();
    }
    self._map.deinit(allocator);
}

pub fn loadFromModel(self: *Self, renderer: *Renderer, model: Model) !Mesh.Handle {
    const vertex_buffer = try convert(renderer.allocator, model);
    var buffer = try renderer.uploadVertexBuffer(vertex_buffer);
    errdefer buffer.deinit();

    const handle: Mesh.Handle = .generate();
    try self._map.put(renderer.allocator, handle, .{
        .buffer = buffer,
    });

    return handle;
}

pub fn loadFromBuffer(self: *Self, renderer: *Renderer, buffer: anytype) !Mesh.Handle {
    const render_buffer = try renderer.uploadVertexBuffer(buffer);

    const handle: Mesh.Handle = .generate();
    try self._map.put(renderer.allocator, handle, .{
        .buffer = render_buffer,
    });

    return handle;
}

pub fn get(self: Self, mesh: Mesh.Handle) ?Mesh {
    return self._map.get(mesh);
}

fn convert(allocator: std.mem.Allocator, model: Model) !VertexBuffer32 {
    var buffer: VertexBuffer32 = try .init(allocator, 0, 0);

    var visited: VisitMap = .init(allocator);
    defer visited.deinit();

    for (model.faces.items) |face| {
        const element1 = face.getTransformed(0) orelse continue;
        const element2 = face.getTransformed(1) orelse continue;
        const element3 = face.getTransformed(2) orelse continue;

        try addElement(allocator, &buffer, &visited, model, element1);
        try addElement(allocator, &buffer, &visited, model, element2);
        try addElement(allocator, &buffer, &visited, model, element3);

        // Swap the indices to ensure winding order.
        const len = buffer.indices.items.len;
        std.mem.swap(u32, &buffer.indices.items[len - 2], &buffer.indices.items[len - 1]);
    }

    return buffer;
}

fn addElement(
    allocator: std.mem.Allocator,
    buffer: *VertexBuffer32,
    visited: *VisitMap,
    model: Model,
    element: Model.Face.Element,
) !void {
    if (visited.get(element)) |index| {
        try buffer.indices.append(allocator, @intCast(index));
    } else {
        const index = buffer.vertices.items.len;
        const vertex = toVertex(element, model) orelse return Error.InvalidModel;
        try buffer.vertices.append(allocator, vertex);
        try buffer.indices.append(allocator, @intCast(index));
        try visited.put(element, index);
    }
}

fn toVertex(element: Model.Face.Element, model: Model) ?Vertex {
    const position = model.getVertex(element.vertex) orelse return null;
    const normal = model.getNormal(element.normal) orelse return null;
    const texture = model.getTextureCoord(element.texture) orelse return null;

    return .init(position, normal, texture, 0xFFFFFFFF);
}
