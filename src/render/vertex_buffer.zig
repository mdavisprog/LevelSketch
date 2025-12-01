const core = @import("core");
const MemFactory = @import("MemFactory.zig");
const std = @import("std");
const zbgfx = @import("zbgfx");

const HexColor = core.math.HexColor;
const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;
const Vec3f = core.math.Vec3f;

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

pub fn VertexBufferBuilder(comptime T: type) type {
    return struct {
        const Self = @This();
        const TVertexBuffer = VertexBuffer(T);

        pub const num_circle_segments: usize = 16;

        buffer: TVertexBuffer,
        vertex_index: T,
        _allocator: std.mem.Allocator,

        pub fn init(allocator: std.mem.Allocator) !Self {
            return .{
                .buffer = try .init(allocator, 0, 0),
                .vertex_index = 0,
                ._allocator = allocator,
            };
        }

        pub fn deinit(self: *Self) void {
            self.buffer.deinit(self._allocator);
        }

        pub fn take(self: *Self) !TVertexBuffer {
            return self.buffer.release(self._allocator);
        }

        pub fn addTri(self: *Self, points: [3]Vertex) !void {
            const v_index = self.buffer.vertices.items.len;
            const i_index = self.buffer.indices.items.len;
            const vertex_index = self.vertex_index;
            try self.buffer.addZeroed(self._allocator, 3, 3);
            try self.buffer.vertices.replaceRange(self._allocator, v_index, points.len, &points);
            try self.buffer.indices.replaceRange(self._allocator, i_index, 3, &.{
                vertex_index + 0,
                vertex_index + 1,
                vertex_index + 2,
            });

            self.*.vertex_index += 3;
        }

        pub fn addQuad(self: *Self, rect: Rectf, abgr: u32) !void {
            const v_index = self.buffer.vertices.items.len;
            const i_index = self.buffer.indices.items.len;
            try self.buffer.addZeroed(self._allocator, 4, 6);

            _ = self.buffer.vertices.items[v_index + 0].setPositionVec2(rect.min);
            _ = self.buffer.vertices.items[v_index + 1].setPositionVec2(.init(rect.min.x, rect.max.y));
            _ = self.buffer.vertices.items[v_index + 2].setPositionVec2(rect.max);
            _ = self.buffer.vertices.items[v_index + 3].setPositionVec2(.init(rect.max.x, rect.min.y));

            _ = self.buffer.vertices.items[v_index + 0].setUV(0.0, 0.0);
            _ = self.buffer.vertices.items[v_index + 1].setUV(0.0, 1.0);
            _ = self.buffer.vertices.items[v_index + 2].setUV(1.0, 1.0);
            _ = self.buffer.vertices.items[v_index + 3].setUV(1.0, 0.0);

            _ = self.buffer.vertices.items[v_index + 0].setColor(abgr);
            _ = self.buffer.vertices.items[v_index + 1].setColor(abgr);
            _ = self.buffer.vertices.items[v_index + 2].setColor(abgr);
            _ = self.buffer.vertices.items[v_index + 3].setColor(abgr);

            self.buffer.indices.items[i_index + 0] = self.vertex_index + 0;
            self.buffer.indices.items[i_index + 1] = self.vertex_index + 1;
            self.buffer.indices.items[i_index + 2] = self.vertex_index + 2;
            self.buffer.indices.items[i_index + 3] = self.vertex_index + 0;
            self.buffer.indices.items[i_index + 4] = self.vertex_index + 2;
            self.buffer.indices.items[i_index + 5] = self.vertex_index + 3;

            self.*.vertex_index += 4;
        }

        pub fn addQuarterArc(self: *Self, center: Vec2f, radius: f32, angle: f32, color: u32) !void {
            var v_index: usize = self.buffer.vertices.items.len;
            var i_index: usize = self.buffer.indices.items.len;

            const center_vertex_index = self.vertex_index;
            self.*.vertex_index += 1;

            try self.buffer.addZeroed(self._allocator, 1, 0);
            self.buffer.vertices.items[v_index] = .init(
                center.x,
                center.y,
                0.0,
                1.0,
                1.0,
                color,
            );

            const segments: f32 = @floatFromInt(num_circle_segments);
            const step: f32 = std.math.degreesToRadians(90.0 / segments);
            const angle_rad: f32 = std.math.degreesToRadians(angle);

            v_index += 1;
            var i: usize = 0;
            while (i < num_circle_segments) {
                defer i += 1;

                const angle_1: f32 = @as(f32, @floatFromInt(i)) * step + angle_rad;
                const angle_2: f32 = @as(f32, @floatFromInt(i + 1)) * step + angle_rad;

                const p1 = center.add(.init(
                    std.math.cos(angle_1) * radius,
                    std.math.sin(angle_1) * radius,
                ));
                const p2 = center.add(.init(
                    std.math.cos(angle_2) * radius,
                    std.math.sin(angle_2) * radius,
                ));

                try self.buffer.addZeroed(self._allocator, 2, 3);

                _ = self.buffer.vertices.items[v_index + 0].setPositionVec2(p1);
                _ = self.buffer.vertices.items[v_index + 1].setPositionVec2(p2);

                _ = self.buffer.vertices.items[v_index + 0].setColor(color);
                _ = self.buffer.vertices.items[v_index + 1].setColor(color);

                _ = self.buffer.vertices.items[v_index + 0].setUV(0.0, 0.0);
                _ = self.buffer.vertices.items[v_index + 1].setUV(1.0, 1.0);

                self.buffer.indices.items[i_index + 0] = center_vertex_index;
                self.buffer.indices.items[i_index + 1] = self.vertex_index + 0;
                self.buffer.indices.items[i_index + 2] = self.vertex_index + 1;

                v_index += 2;
                i_index += 3;
                self.*.vertex_index += 2;
            }
        }
    };
}
