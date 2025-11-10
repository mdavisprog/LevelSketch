const std = @import("std");
const Vertex = @import("Vertex.zig");

pub fn VertexBuffer(comptime IndexType: type) type {
    if (IndexType != u16 and IndexType != u32) {
        @compileError("VertexBuffer must be of type u16 or u32.");
    }

    return struct {
        const Self = @This();

        vertices: []Vertex,
        indices: []IndexType,

        pub fn init(allocator: std.mem.Allocator, vertex_count: usize, index_count: usize) !Self {
            const vertices = try allocator.alloc(Vertex, vertex_count);
            const indices = try allocator.alloc(u16, index_count);
            return Self{
                .vertices = vertices,
                .indices = indices,
            };
        }

        pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
            allocator.free(self.vertices);
            allocator.free(self.indices);
        }
    };
}
