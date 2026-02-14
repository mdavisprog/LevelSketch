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
const Vec = core.math.Vec;
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
        var first_index: u32 = 0;
        for (0..face.elements.items.len) |element_index| {
            const element = face.getTransformed(element_index) orelse return Error.InvalidModel;
            try addElement(allocator, &buffer, &visited, model, element);

            // Keep track of the first element for this face. This is needed for faces with more
            // than three vertices e.g. quads.
            if (element_index == 0) {
                first_index = buffer.indices.items[buffer.indices.items.len - 1];
            }

            if (element_index >= 2) {
                // If this face is not a triangle, then need to generate a triangle to behave like
                // a triangle fan using the first element of this face as the pivot.
                if (element_index > 2) {
                    // Need to account for the indices swapped from the previous triangle.
                    const other = buffer.indices.items[buffer.indices.items.len - 3];
                    try buffer.indices.append(allocator, first_index);
                    try buffer.indices.append(allocator, other);
                }

                // The final two elements need to be swapped for proper winding order.
                const len = buffer.indices.items.len;
                std.mem.swap(u32, &buffer.indices.items[len - 2], &buffer.indices.items[len - 1]);
            }
        }
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
    // Normal and texture coordinates are optional for faces. If not specified, mark them as
    // zero.
    const normal = model.getNormal(element.normal) orelse Vec.zero;
    const texture = model.getTextureCoord(element.texture) orelse Vec.zero;

    return .init(position, normal, texture, 0xFFFFFFFF);
}
