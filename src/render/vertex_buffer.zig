const MemFactory = @import("MemFactory.zig");
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

        // TODO: Find a better way to handle memory uploads and proper freeing.
        _vertices_allocated: bool = false,
        _indices_allocated: bool = false,

        pub fn init(allocator: std.mem.Allocator, vertex_count: usize, index_count: usize) !Self {
            const vertices = try allocator.alloc(Vertex, vertex_count);
            const indices = try allocator.alloc(u16, index_count);
            return Self{
                .vertices = vertices,
                .indices = indices,
                ._vertices_allocated = true,
                ._indices_allocated = true,
            };
        }

        pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
            if (self._vertices_allocated) {
                allocator.free(self.vertices);
                self.*._vertices_allocated = false;
            }

            if (self._indices_allocated) {
                allocator.free(self.indices);
                self.*._indices_allocated = false;
            }
        }

        pub fn createMemVertex(self: *Self, factory: *MemFactory) !MemFactory.Mem {
            return try factory.create(
                @ptrCast(self.vertices),
                onUploadedVertex,
                @ptrCast(self),
            );
        }

        pub fn createMemIndex(self: *Self, factory: *MemFactory) !MemFactory.Mem {
            return try factory.create(
                @ptrCast(self.indices),
                onUploadedIndex,
                @ptrCast(self),
            );
        }

        fn onUploadedVertex(result: MemFactory.OnUploadedResult) void {
            const self: *Self = @ptrCast(@alignCast(result.user_data.?));
            result.allocator.free(self.vertices);
            self._vertices_allocated = false;
        }

        fn onUploadedIndex(result: MemFactory.OnUploadedResult) void {
            const self: *Self = @ptrCast(@alignCast(result.user_data.?));
            result.allocator.free(self.indices);
            self._indices_allocated = false;
        }
    };
}
