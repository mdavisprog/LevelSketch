const io = @import("io");
const render = @import("root.zig");
const std = @import("std");

const Model = io.obj.Model;

const RenderBuffer = render.RenderBuffer;
const Renderer = render.Renderer;
const Texture = render.Texture;
const Vertex = render.Vertex;
const VertexBuffer16 = render.VertexBuffer16;
const VertexBuffer32 = render.VertexBuffer32;

const Self = @This();
const VisitMap = std.AutoHashMap(Model.Face.Element, usize);

pub const Error = error{
    InvalidModel,
};

buffer: RenderBuffer,
texture: Texture = .{},

pub fn init(renderer: *Renderer, model: Model) !Self {
    const vertex_buffer = try convert(renderer.allocator, model);
    const buffer = try renderer.uploadVertexBuffer(vertex_buffer);
    errdefer buffer.deinit();

    // TODO: Load all materials.
    const texture: Texture = blk: {
        if (model.materials.items.len > 0) {
            if (model.materials.items[0].diffuse_texture) |path| {
                const texture = renderer.textures.loadImageAbsolute(
                    &renderer.mem_factory,
                    path,
                ) catch |err| {
                    std.debug.print(
                        "Failed to load texture '{s}'' from model. Error: {}\n",
                        .{ path, err },
                    );
                    break :blk .{};
                };

                break :blk texture;
            }
        }

        break :blk .{};
    };

    return .{
        .buffer = buffer,
        .texture = texture,
    };
}

pub fn initWithBuffer(
    renderer: *Renderer,
    buffer: anytype,
) !Self {
    const mesh_buffer = try renderer.uploadVertexBuffer(buffer);
    return .{
        .buffer = mesh_buffer,
    };
}

pub fn deinit(self: *Self) void {
    self.buffer.deinit();
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
