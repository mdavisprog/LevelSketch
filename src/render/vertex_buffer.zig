const MemFactory = @import("MemFactory.zig");
const std = @import("std");
const Vertex = @import("Vertex.zig");

pub fn VertexBuffer(comptime IndexType: type) type {
    if (IndexType != u16 and IndexType != u32) {
        @compileError("VertexBuffer must be of type u16 or u32.");
    }

    return struct {
        const Self = @This();

        vertices: std.ArrayList(Vertex),
        indices: std.ArrayList(IndexType),

        pub fn init(allocator: std.mem.Allocator, vertex_count: usize, index_count: usize) !Self {
            var vertices = try std.ArrayList(Vertex).initCapacity(allocator, vertex_count);
            try vertices.appendNTimes(allocator, .{}, vertex_count);

            var indices = try std.ArrayList(u16).initCapacity(allocator, index_count);
            try indices.appendNTimes(allocator, 0, index_count);

            return Self{
                .vertices = vertices,
                .indices = indices,
            };
        }

        pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
            self.vertices.deinit(allocator);
            self.indices.deinit(allocator);
        }

        pub fn append(self: *Self, allocator: std.mem.Allocator, from: Self) !void {
            try self.vertices.appendSlice(allocator, from.vertices.items);
            try self.indices.appendSlice(allocator, from.indices.items);
        }

        pub fn createMemVertex(self: *Self, factory: *MemFactory) !MemFactory.Mem {
            return try factory.create(
                @ptrCast(self.vertices.items),
                onUploadedVertex,
                @ptrCast(self),
            );
        }

        /// Don't track the memory allocation. Caller is responsible for freeing the memory.
        pub fn createMemVertexTransient(self: Self, factory: *MemFactory) !MemFactory.Mem {
            return try factory.create(@ptrCast(self.vertices.items), null, null);
        }

        pub fn createMemIndex(self: *Self, factory: *MemFactory) !MemFactory.Mem {
            return try factory.create(
                @ptrCast(self.indices.items),
                onUploadedIndex,
                @ptrCast(self),
            );
        }

        /// Don't track the memory allocation. Caller is responsible for freeing the memory.
        pub fn createMemIndexTransient(self: Self, factory: *MemFactory) !MemFactory.Mem {
            return try factory.create(@ptrCast(self.indices.items), null, null);
        }

        fn onUploadedVertex(result: MemFactory.OnUploadedResult) void {
            const self: *Self = @ptrCast(@alignCast(result.user_data.?));
            self.vertices.clearAndFree(result.allocator);
        }

        fn onUploadedIndex(result: MemFactory.OnUploadedResult) void {
            const self: *Self = @ptrCast(@alignCast(result.user_data.?));
            self.indices.clearAndFree(result.allocator);
        }
    };
}
