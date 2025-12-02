const io = @import("io");
const render = @import("root.zig");
const std = @import("std");

const Model = io.obj.Model;

const RenderBuffer = render.RenderBuffer;
const Renderer = render.Renderer;
const Vertex = render.Vertex;
const VertexBuffer32 = render.VertexBuffer32;
const VertexBufferBuilder32 = render.VertexBufferBuilder32;

const Self = @This();
const VisitMap = std.AutoHashMap(Model.Face.Element, usize);

pub const Error = error{
    InvalidModel,
};

buffer: RenderBuffer,

pub fn init(renderer: *Renderer, model: Model) !Self {
    const vertex_buffer = try convert(renderer.allocator, model);
    const buffer = try renderer.uploadVertexBuffer(vertex_buffer);
    return .{
        .buffer = buffer,
    };
}

pub fn deinit(self: *Self) void {
    self.buffer.deinit();
}

fn convert(allocator: std.mem.Allocator, model: Model) !VertexBuffer32 {
    var builder: VertexBufferBuilder32 = try .init(allocator);
    defer builder.deinit();

    var visited: VisitMap = .init(allocator);
    defer visited.deinit();

    for (model.faces.items) |face| {
        const element1 = face.getTransformed(0) orelse continue;
        const element2 = face.getTransformed(1) orelse continue;
        const element3 = face.getTransformed(2) orelse continue;

        try addElement(allocator, &builder, &visited, model, element1);
        try addElement(allocator, &builder, &visited, model, element2);
        try addElement(allocator, &builder, &visited, model, element3);
    }

    return try builder.take();
}

fn addElement(
    allocator: std.mem.Allocator,
    builder: *VertexBufferBuilder32,
    visited: *VisitMap,
    model: Model,
    element: Model.Face.Element,
) !void {
    if (visited.get(element)) |index| {
        try builder.buffer.indices.append(allocator, @intCast(index));
    } else {
        const index = builder.vertex_index;
        const vertex = toVertex(element, model) orelse return Error.InvalidModel;
        try builder.buffer.vertices.append(allocator, vertex);
        try builder.buffer.indices.append(allocator, @intCast(index));
        try visited.put(element, index);
        builder.vertex_index += 1;
    }
}

fn toVertex(element: Model.Face.Element, model: Model) ?Vertex {
    const position = model.getVertex(element.vertex) orelse return null;
    const texture = model.getTextureCoord(element.texture) orelse return null;

    return .init(position.x, position.y, position.z, texture.x, texture.y, 0xFFFFFFFF);
}
