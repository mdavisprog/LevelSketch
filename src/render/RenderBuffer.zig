const MemFactory = @import("MemFactory.zig");
const render = @import("root.zig");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Vertex = render.Vertex;
const VertexBuffer16 = render.VertexBuffer16;
const VertexBuffer32 = render.VertexBuffer32;
const UIVertex = render.UIVertex;
const UIVertexBuffer16 = render.UIVertexBuffer16;
const UIVertexBuffer32 = render.UIVertexBuffer32;

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

pub const Callbacks = struct {
    on_upload_vertices: ?MemFactory.OnUploaded = null,
    on_upload_indices: ?MemFactory.OnUploaded = null,
    user_data: ?*anyopaque = null,
};

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

pub fn setStaticBuffer(
    self: *Self,
    factory: *MemFactory,
    buffer: anytype,
    callbacks: ?Callbacks,
) !void {
    try self.set(factory, .static, buffer, callbacks);
}

pub fn setTransientBuffer(self: *Self, buffer: anytype) !void {
    try self.set(null, .transient, buffer, null);
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

fn set(
    self: *Self,
    factory: ?*MemFactory,
    buffer_type: BufferTypeTag,
    buffer: anytype,
    callbacks: ?Callbacks,
) !void {
    if (self.vertex != null) return Error.AlreadyInitialized;
    if (self.index != null) return Error.AlreadyInitialized;

    const BufferType = @TypeOf(buffer);
    const layout: zbgfx.bgfx.VertexLayout = switch (BufferType) {
        VertexBuffer16, VertexBuffer32 => Vertex.Layout.init().data,
        UIVertexBuffer16, UIVertexBuffer32 => UIVertex.Layout.init().data,
        else => emitInvalidTypeError(BufferType),
    };

    const is_u32 = switch (BufferType) {
        VertexBuffer16, UIVertexBuffer16 => false,
        VertexBuffer32, UIVertexBuffer32 => true,
        else => emitInvalidTypeError(BufferType),
    };
    const flags = if (is_u32)
        zbgfx.bgfx.BufferFlags_Index32
    else
        zbgfx.bgfx.BufferFlags_None;

    self.vertex_len = @intCast(buffer.vertices.items.len);
    self.index_len = @intCast(buffer.indices.items.len);

    const on_upload_vertices = if (callbacks) |callbacks_|
        callbacks_.on_upload_vertices
    else
        null;

    const on_upload_indices = if (callbacks) |callbacks_|
        callbacks_.on_upload_indices
    else
        null;

    const user_data = if (callbacks) |callbacks_| callbacks_.user_data else null;

    const v_data: []const u8 = @ptrCast(buffer.vertices.items);
    const i_data: []const u8 = @ptrCast(buffer.indices.items);

    switch (buffer_type) {
        .static => {
            const v_mem = try factory.?.create(v_data, on_upload_vertices, user_data);
            const v_handle = zbgfx.bgfx.createVertexBuffer(
                v_mem.ptr,
                &layout,
                zbgfx.bgfx.BufferFlags_None,
            );
            self.vertex = .{ .static = v_handle };

            const i_mem = try factory.?.create(i_data, on_upload_indices, user_data);
            const i_handle = zbgfx.bgfx.createIndexBuffer(
                i_mem.ptr,
                flags,
            );
            self.index = .{ .static = i_handle };
        },
        .dynamic => {
            const v_mem = try factory.?.create(v_data, on_upload_vertices, user_data);
            const v_handle = zbgfx.bgfx.createDynamicVertexBufferMem(
                v_mem.ptr,
                &layout,
                zbgfx.bgfx.BufferFlags_None,
            );
            self.vertex = .{ .dynamic = v_handle };

            const i_mem = try factory.?.create(i_data, on_upload_indices, user_data);
            const i_handle = zbgfx.bgfx.createDynamicIndexBufferMem(
                i_mem.ptr,
                flags,
            );
            self.index = .{ .dynamic = i_handle };
        },
        .transient => {
            var v_handle: zbgfx.bgfx.TransientVertexBuffer = undefined;
            zbgfx.bgfx.allocTransientVertexBuffer(@ptrCast(&v_handle), self.vertex_len, &layout);
            self.vertex = .{ .transient = v_handle };

            const v_dst = v_handle.data[0..v_handle.size];
            @memcpy(v_dst, v_data.ptr);

            var i_handle: zbgfx.bgfx.TransientIndexBuffer = undefined;
            zbgfx.bgfx.allocTransientIndexBuffer(@ptrCast(&i_handle), self.index_len, is_u32);
            self.index = .{ .transient = i_handle };

            const i_dst = i_handle.data[0..i_handle.size];
            @memcpy(i_dst, i_data.ptr);
        },
    }
}

fn emitInvalidTypeError(comptime BufferType: type) void {
    @compileError(std.fmt.comptimePrint(
        "Invalid buffer type '{s}'. Must be of type VertexBuffer or UIVertexBuffer.",
        .{@typeName(BufferType)},
    ));
}
