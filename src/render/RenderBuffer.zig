const std = @import("std");
const Vertex = @import("Vertex.zig");
const zbgfx = @import("zbgfx");

const Self = @This();

vertex: ?zbgfx.bgfx.VertexBufferHandle = null,
vertex_len: u32 = 0,
index: ?zbgfx.bgfx.IndexBufferHandle = null,
index_len: u32 = 0,

pub fn init() Self {
    return Self{};
}

pub fn deinit(self: *Self) void {
    if (self.vertex) |vertex| zbgfx.bgfx.destroyVertexBuffer(vertex);
    if (self.index) |index| zbgfx.bgfx.destroyIndexBuffer(index);
}

pub fn setVertices(self: *Self, vertices: []const Vertex) void {
    std.debug.assert(self.vertex == null);

    const layout = Vertex.Layout.init();
    self.vertex = zbgfx.bgfx.createVertexBuffer(
        zbgfx.bgfx.makeRef(vertices.ptr, @intCast(vertices.len * @sizeOf(Vertex))),
        &layout.data,
        zbgfx.bgfx.BufferFlags_None,
    );
    self.vertex_len = @intCast(vertices.len);
}

pub fn setIndices16(self: *Self, indices: []const u16) void {
    std.debug.assert(self.index == null);

    self.index = zbgfx.bgfx.createIndexBuffer(
        zbgfx.bgfx.makeRef(indices.ptr, @intCast(indices.len * @sizeOf(u16))),
        zbgfx.bgfx.BufferFlags_None,
    );
    self.index_len = @intCast(indices.len);
}

pub fn bind(self: Self, state: u64) void {
    std.debug.assert(self.vertex != null);
    std.debug.assert(self.index != null);

    zbgfx.bgfx.setVertexBuffer(0, self.vertex.?, 0, self.vertex_len);
    zbgfx.bgfx.setIndexBuffer(self.index.?, 0, self.index_len);
    zbgfx.bgfx.setState(state, 0);
}
