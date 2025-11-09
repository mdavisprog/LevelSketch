const MemFactory = @import("MemFactory.zig");
const std = @import("std");
const Vertex = @import("Vertex.zig");
const zbgfx = @import("zbgfx");

pub const Error = error{
    NotInitialized,
    AlreadyInitialized,
    CantUpdateStaticBuffer,
};

pub const BufferTypeTag = enum {
    static,
    dynamic,
};

pub const VertexHandle = union(BufferTypeTag) {
    static: zbgfx.bgfx.VertexBufferHandle,
    dynamic: zbgfx.bgfx.DynamicVertexBufferHandle,
};

pub const IndexHandle = union(BufferTypeTag) {
    static: zbgfx.bgfx.IndexBufferHandle,
    dynamic: zbgfx.bgfx.DynamicIndexBufferHandle,
};

const Self = @This();

vertex: ?VertexHandle = null,
vertex_len: u32 = 0,
index: ?IndexHandle = null,
index_len: u32 = 0,

pub fn init() Self {
    return Self{};
}

pub fn deinit(self: *Self) void {
    if (self.vertex) |vertex| {
        switch (vertex) {
            .static => |handle| {
                zbgfx.bgfx.destroyVertexBuffer(handle);
            },
            .dynamic => |handle| {
                zbgfx.bgfx.destroyDynamicVertexBuffer(handle);
            },
        }
    }

    if (self.index) |index| {
        switch (index) {
            .static => |handle| {
                zbgfx.bgfx.destroyIndexBuffer(handle);
            },
            .dynamic => |handle| {
                zbgfx.bgfx.destroyDynamicIndexBuffer(handle);
            },
        }
    }

    self.vertex = null;
    self.index = null;
    self.vertex_len = 0;
    self.index_len = 0;
}

pub fn setStaticVertices(self: *Self, mem: [*c]const zbgfx.bgfx.Memory, length: usize) !void {
    if (self.vertex != null) return Error.AlreadyInitialized;

    const layout = Vertex.Layout.init();
    const handle = zbgfx.bgfx.createVertexBuffer(
        mem,
        &layout.data,
        zbgfx.bgfx.BufferFlags_None,
    );
    self.vertex = .{ .static = handle };
    self.vertex_len = @intCast(length);
}

pub fn setDynamicVertices(self: *Self, mem: [*c]const zbgfx.bgfx.Memory, length: usize) !void {
    if (self.vertex != null) return Error.AlreadyInitialized;

    const layout = Vertex.Layout.init();
    const handle = zbgfx.bgfx.createDynamicVertexBufferMem(
        mem,
        &layout.data,
        zbgfx.bgfx.BufferFlags_None,
    );
    self.vertex = .{ .dynamic = handle };
    self.vertex_len = @intCast(length);
}

pub fn setStaticIndices(self: *Self, mem: [*c]const zbgfx.bgfx.Memory, length: usize) !void {
    if (self.index != null) return Error.AlreadyInitialized;

    const handle = zbgfx.bgfx.createIndexBuffer(
        mem,
        zbgfx.bgfx.BufferFlags_None,
    );
    self.index = .{ .static = handle };
    self.index_len = @intCast(length);
}

pub fn setDynamicIndices(self: *Self, mem: [*c]const zbgfx.bgfx.Memory, length: usize) !void {
    if (self.index != null) return Error.AlreadyInitialized;

    const handle = zbgfx.bgfx.createDynamicIndexBufferMem(
        mem,
        zbgfx.bgfx.BufferFlags_None,
    );
    self.index = .{ .dynamic = handle };
    self.index_len = @intCast(length);
}

pub fn updateVertices(self: Self, start_vertex: u32, mem: [*c]const zbgfx.bgfx.Memory) !void {
    if (self.vertex == null) return Error.NotInitialized;

    if (self.vertex) |vertex| {
        switch (vertex) {
            .static => |_| {
                return Error.CantUpdateStaticBuffer;
            },
            .dynamic => |handle| {
                zbgfx.bgfx.updateDynamicVertexBuffer(handle, start_vertex, mem);
            },
        }
    }
}

pub fn updateIndices(self: Self, start_index: u32, mem: [*c]const zbgfx.bgfx.Memory) !void {
    if (self.index == null) return Error.NotInitialized;

    if (self.index) |index| {
        switch (index) {
            .static => |_| {
                return Error.CantUpdateStaticBuffer;
            },
            .dynamic => |handle| {
                zbgfx.bgfx.updateDynamicIndexBuffer(handle, start_index, mem);
            },
        }
    }
}

pub fn bind(self: Self, state: u64) void {
    std.debug.assert(self.vertex != null);
    std.debug.assert(self.index != null);

    if (self.vertex) |vertex| {
        switch (vertex) {
            .static => |handle| {
                zbgfx.bgfx.setVertexBuffer(0, handle, 0, self.vertex_len);
            },
            .dynamic => |handle| {
                zbgfx.bgfx.setDynamicVertexBuffer(0, handle, 0, self.vertex_len);
            },
        }
    }

    if (self.index) |index| {
        switch (index) {
            .static => |handle| {
                zbgfx.bgfx.setIndexBuffer(handle, 0, self.index_len);
            },
            .dynamic => |handle| {
                zbgfx.bgfx.setDynamicIndexBuffer(handle, 0, self.index_len);
            },
        }
    }

    zbgfx.bgfx.setState(state, 0);
}
