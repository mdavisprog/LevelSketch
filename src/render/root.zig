pub const Atlas = @import("Atlas.zig");
pub const Commands = @import("Commands.zig");
pub const Font = @import("Font.zig");
pub const Fonts = @import("Fonts.zig");
pub const materials = @import("materials/root.zig");
pub const MemFactory = @import("MemFactory.zig");
pub const Meshes = @import("Meshes.zig");
pub const RenderBuffer = @import("RenderBuffer.zig");
pub const Renderer = @import("Renderer.zig");
pub const shaders = @import("shaders/root.zig");
pub const shapes = @import("shapes.zig");
pub const Texture = @import("Texture.zig");
pub const Textures = @import("Textures.zig");
pub const View = @import("View.zig");

const std = @import("std");
const vertex_buffer = @import("vertex_buffer.zig");

pub const Vertex = vertex_buffer.Vertex;
pub const VertexBuffer16 = vertex_buffer.VertexBuffer(Vertex, u16);
pub const VertexBuffer32 = vertex_buffer.VertexBuffer(Vertex, u32);
pub const VertexBufferUploads16 = vertex_buffer.VertexBufferUploads(Vertex, u16);
pub const VertexBufferUploads32 = vertex_buffer.VertexBufferUploads(Vertex, u32);

pub const UIVertex = vertex_buffer.UIVertex;
pub const UIVertexBuffer16 = vertex_buffer.VertexBuffer(UIVertex, u16);
pub const UIVertexBuffer32 = vertex_buffer.VertexBuffer(UIVertex, u32);

pub fn stateFlagsBlend(src: u64, dst: u64) u64 {
    return (src | (dst << 4)) | ((src | (dst << 4)) << 8);
}

pub fn Handle(comptime T: type) type {
    return struct {
        const Self = @This();

        pub const Id = u32;
        pub const invalid_id: Id = std.math.maxInt(Id);
        pub const invalid: Self = .init(invalid_id);

        const Type = struct {
            const name = @typeName(T);
            var id: Id = 0;
        };

        id: Id = invalid_id,

        pub fn init(id: Id) Self {
            return .{
                .id = id,
            };
        }

        pub fn generate() Self {
            const id = Type.id;
            Type.id += 1;
            return .init(id);
        }

        pub fn eql(self: Self, other: Self) bool {
            return self.id == other.id;
        }

        pub fn isValid(self: Self) bool {
            return !self.eql(invalid);
        }

        pub fn typeName(_: Self) []const u8 {
            return Type.name;
        }
    };
}

test "refall" {
    std.testing.refAllDecls(@This());
}

test "handles" {
    const TestStruct1 = struct {};
    const TestStruct2 = struct {};

    const TestHandle1 = Handle(TestStruct1);
    const TestHandle2 = Handle(TestStruct2);

    const handle1: TestHandle1 = .generate();
    const handle2: TestHandle2 = .generate();

    try std.testing.expectEqual(0, handle1.id);
    try std.testing.expectEqual(0, handle2.id);
}
