const MemFactory = @import("MemFactory.zig");
const render = @import("root.zig");
const std = @import("std");
const Vertex = @import("Vertex.zig");
const zbgfx = @import("zbgfx");

const VertexBuffer16 = render.VertexBuffer16;

pub const Error = error{
    NotInitialized,
    AlreadyInitialized,
    CantUpdateBuffer,
};

pub const BufferTypeTag = enum {
    static,
    dynamic,
    transient,
};

pub const VertexHandle = union(BufferTypeTag) {
    static: zbgfx.bgfx.VertexBufferHandle,
    dynamic: zbgfx.bgfx.DynamicVertexBufferHandle,
    transient: zbgfx.bgfx.TransientVertexBuffer,
};

pub const IndexHandle = union(BufferTypeTag) {
    static: zbgfx.bgfx.IndexBufferHandle,
    dynamic: zbgfx.bgfx.DynamicIndexBufferHandle,
    transient: zbgfx.bgfx.TransientIndexBuffer,
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
            .transient => {},
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
            .transient => {},
        }
    }

    self.vertex = null;
    self.index = null;
    self.vertex_len = 0;
    self.index_len = 0;
}

pub fn setStaticVertices(self: *Self, mem: MemFactory.Mem, length: usize) !void {
    if (self.vertex != null) return Error.AlreadyInitialized;

    const layout = Vertex.Layout.init();
    const handle = zbgfx.bgfx.createVertexBuffer(
        mem.ptr,
        &layout.data,
        zbgfx.bgfx.BufferFlags_None,
    );
    self.vertex = .{ .static = handle };
    self.vertex_len = @intCast(length);
}

pub fn setDynamicVertices(self: *Self, mem: MemFactory.Mem, length: usize) !void {
    if (self.vertex != null) return Error.AlreadyInitialized;

    const layout = Vertex.Layout.init();
    const handle = zbgfx.bgfx.createDynamicVertexBufferMem(
        mem.ptr,
        &layout.data,
        zbgfx.bgfx.BufferFlags_None,
    );
    self.vertex = .{ .dynamic = handle };
    self.vertex_len = @intCast(length);
}

/// Will copy the contents of mem into the transient buffer.
pub fn setTransientVertices(self: *Self, mem: MemFactory.Mem, length: usize) !void {
    if (self.vertex != null) return Error.AlreadyInitialized;

    const layout = Vertex.Layout.init();
    var handle: zbgfx.bgfx.TransientVertexBuffer = undefined;
    zbgfx.bgfx.allocTransientVertexBuffer(@ptrCast(&handle), @intCast(length), &layout.data);
    self.vertex = .{ .transient = handle };
    self.vertex_len = @intCast(length);

    const dst = handle.data[0..handle.size];
    @memcpy(dst, mem.ptr.*.data);
}

pub fn setStaticIndices(self: *Self, mem: MemFactory.Mem, length: usize) !void {
    if (self.index != null) return Error.AlreadyInitialized;

    const handle = zbgfx.bgfx.createIndexBuffer(
        mem.ptr,
        zbgfx.bgfx.BufferFlags_None,
    );
    self.index = .{ .static = handle };
    self.index_len = @intCast(length);
}

pub fn setDynamicIndices(self: *Self, mem: MemFactory.Mem, length: usize) !void {
    if (self.index != null) return Error.AlreadyInitialized;

    const handle = zbgfx.bgfx.createDynamicIndexBufferMem(
        mem.ptr,
        zbgfx.bgfx.BufferFlags_None,
    );
    self.index = .{ .dynamic = handle };
    self.index_len = @intCast(length);
}

/// Will copy the contents of mem into the transient buffer.
pub fn setTransientIndices(self: *Self, mem: MemFactory.Mem, length: usize) !void {
    if (self.index != null) return Error.AlreadyInitialized;

    var handle: zbgfx.bgfx.TransientIndexBuffer = undefined;
    const alignment = mem.ptr.*.size / length;
    const is_u32 = alignment == @sizeOf(u32);
    zbgfx.bgfx.allocTransientIndexBuffer(@ptrCast(&handle), @intCast(length), is_u32);
    self.index = .{ .transient = handle };
    self.index_len = @intCast(length);

    const dst = handle.data[0..handle.size];
    @memcpy(dst, mem.ptr.*.data);
}

pub fn setStaticBuffer(self: *Self, factory: *MemFactory, buffer: *VertexBuffer16) !void {
    const v_mem = try buffer.createMemVertex(factory);
    const i_mem = try buffer.createMemIndex(factory);
    try self.setStaticVertices(v_mem, buffer.vertices.items.len);
    try self.setStaticIndices(i_mem, buffer.indices.items.len);
}

pub fn setTransientBuffer(self: *Self, factory: *MemFactory, buffer: VertexBuffer16) !void {
    const v_mem = try buffer.createMemVertexTransient(factory);
    const i_mem = try buffer.createMemIndexTransient(factory);
    try self.setTransientVertices(v_mem, buffer.vertices.items.len);
    try self.setTransientVertices(i_mem, buffer.indices.items.len);
}

pub fn updateVertices(self: Self, start_vertex: u32, mem: MemFactory.Mem) !void {
    if (self.vertex == null) return Error.NotInitialized;

    if (self.vertex) |vertex| {
        switch (vertex) {
            .static, .transient => |_| {
                return Error.CantUpdateBuffer;
            },
            .dynamic => |handle| {
                zbgfx.bgfx.updateDynamicVertexBuffer(handle, start_vertex, mem.ptr);
            },
        }
    }
}

pub fn updateIndices(self: Self, start_index: u32, mem: MemFactory.Mem) !void {
    if (self.index == null) return Error.NotInitialized;

    if (self.index) |index| {
        switch (index) {
            .static, .transient => |_| {
                return Error.CantUpdateBuffer;
            },
            .dynamic => |handle| {
                zbgfx.bgfx.updateDynamicIndexBuffer(handle, start_index, mem.ptr);
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
            .transient => |handle| {
                zbgfx.bgfx.setTransientVertexBuffer(0, @ptrCast(&handle), 0, self.vertex_len);
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
            .transient => |handle| {
                zbgfx.bgfx.setTransientIndexBuffer(@ptrCast(&handle), 0, self.index_len);
            },
        }
    }

    zbgfx.bgfx.setState(state, 0);
}

pub fn isValid(self: Self) bool {
    return self.vertex != null and self.index != null;
}
