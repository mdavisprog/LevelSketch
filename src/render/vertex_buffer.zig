const core = @import("core");
const render = @import("root.zig");
const std = @import("std");
const zbgfx = @import("zbgfx");

const HexColor = core.math.HexColor;
const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;
const Vec3f = core.math.Vec3f;

const MemFactory = render.MemFactory;
const RenderBuffer = render.RenderBuffer;

pub const Vertex = extern struct {
    const Self = @This();

    pub const Layout = extern struct {
        data: zbgfx.bgfx.VertexLayout,

        pub fn init() Layout {
            var data = std.mem.zeroes(zbgfx.bgfx.VertexLayout);
            data.begin(zbgfx.bgfx.RendererType.Noop)
                .add(.Position, 3, .Float, false, false)
                .add(.TexCoord0, 2, .Float, true, false)
                .add(.Color0, 4, .Uint8, true, true)
                .end();
            return .{
                .data = data,
            };
        }
    };

    x: f32 = 0.0,
    y: f32 = 0.0,
    z: f32 = 0.0,
    u: f32 = 0.0,
    v: f32 = 0.0,
    abgr: u32 = 0xFFFFFFFF,

    pub fn init(x: f32, y: f32, z: f32, u: f32, v: f32, abgr: u32) Self {
        return .{
            .x = x,
            .y = y,
            .z = z,
            .u = u,
            .v = v,
            .abgr = abgr,
        };
    }

    pub fn setPosition(self: *Self, x: f32, y: f32, z: f32) *Self {
        self.x = x;
        self.y = y;
        self.z = z;
        return self;
    }

    pub fn setPositionVec3(self: *Self, position: Vec3f) *Self {
        self.x = position.x;
        self.y = position.y;
        self.z = position.z;
        return self;
    }

    pub fn setPositionVec2(self: *Self, position: Vec2f) *Self {
        self.x = position.x;
        self.y = position.y;
        self.z = 0.0;
        return self;
    }

    pub fn setUV(self: *Self, u: f32, v: f32) *Self {
        self.u = u;
        self.v = v;
        return self;
    }

    pub fn setUVVec2(self: *Self, uv: Vec2f) *Self {
        self.u = uv.x;
        self.v = uv.y;
        return self;
    }

    pub fn setColor(self: *Self, abgr: u32) *Self {
        self.abgr = abgr;
        return self;
    }

    pub fn setColor4b(self: *Self, r: u8, g: u8, b: u8, a: u8) *Self {
        const hex: HexColor = .init(r, g, b, a, .abgr);
        self.abgr = hex.data;
        return self;
    }
};

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

            var indices = try std.ArrayList(IndexType).initCapacity(allocator, index_count);
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

        pub fn addZeroed(
            self: *Self,
            allocator: std.mem.Allocator,
            vertices: usize,
            indices: usize,
        ) !void {
            try self.vertices.appendNTimes(allocator, .{}, vertices);
            try self.indices.appendNTimes(allocator, 0, indices);
        }

        pub fn createMemVertex(self: *Self, factory: *MemFactory) !MemFactory.Mem {
            return try factory.create(
                @ptrCast(self.vertices.items),
                onUploadedVertex,
                @ptrCast(self),
            );
        }

        pub fn createMemIndex(self: *Self, factory: *MemFactory) !MemFactory.Mem {
            return try factory.create(
                @ptrCast(self.indices.items),
                onUploadedIndex,
                @ptrCast(self),
            );
        }

        pub fn release(self: *Self, allocator: std.mem.Allocator) !Self {
            const vertices: std.ArrayList(Vertex) =
                .fromOwnedSlice(try self.vertices.toOwnedSlice(allocator));
            const indices: std.ArrayList(IndexType) =
                .fromOwnedSlice(try self.indices.toOwnedSlice(allocator));

            return .{
                .vertices = vertices,
                .indices = indices,
            };
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

pub fn VertexBufferUploads(comptime IndexType: type) type {
    return struct {
        const Self = @This();
        const VertexBufferType = VertexBuffer(IndexType);

        const Upload = struct {
            vertex_complete: bool = false,
            index_complete: bool = false,
            buffer: VertexBufferType,

            fn is_complete(self: Upload) bool {
                return self.vertex_complete and self.index_complete;
            }
        };

        uploads: std.ArrayList(*Upload),

        pub fn init(allocator: std.mem.Allocator) !Self {
            return .{
                .uploads = try .initCapacity(allocator, 0),
            };
        }

        pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
            for (self.uploads.items) |upload| {
                upload.buffer.deinit(allocator);
                allocator.destroy(upload);
            }

            self.uploads.deinit(allocator);
        }

        pub fn addUpload(
            self: *Self,
            mem_factory: *MemFactory,
            buffer: VertexBufferType,
        ) !RenderBuffer {
            const allocator = mem_factory.allocator;

            const upload: *Upload = try allocator.create(Upload);
            upload.buffer = buffer;
            try self.uploads.append(allocator, upload);

            const v_mem = try mem_factory.create(
                @ptrCast(buffer.vertices.items),
                onUploadVertices,
                upload,
            );
            const i_mem = try mem_factory.create(
                @ptrCast(buffer.indices.items),
                onUploadIndices,
                upload,
            );

            var result: RenderBuffer = .init();
            try result.setStaticVertices(v_mem, buffer.vertices.items.len);
            try result.setStaticIndices(i_mem, buffer.indices.items.len);
            return result;
        }

        pub fn update(self: *Self, allocator: std.mem.Allocator) void {
            var i: usize = 0;
            while (i < self.uploads.items.len) {
                if (self.uploads.items[i].is_complete()) {
                    const upload = self.uploads.swapRemove(i);

                    upload.buffer.deinit(allocator);
                    allocator.destroy(upload);
                } else {
                    i += 1;
                }
            }
        }

        fn onUploadVertices(result: MemFactory.OnUploadedResult) void {
            const upload: *Upload = result.userDataAs(Upload);
            upload.vertex_complete = true;
        }

        fn onUploadIndices(result: MemFactory.OnUploadedResult) void {
            const upload: *Upload = result.userDataAs(Upload);
            upload.index_complete = true;
        }
    };
}
