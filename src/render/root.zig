pub const Atlas = @import("Atlas.zig");
pub const Commands = @import("Commands.zig");
pub const ecs = @import("ecs/root.zig");
pub const Font = @import("Font.zig");
pub const Fonts = @import("Fonts.zig");
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

test "refall" {
    std.testing.refAllDecls(@This());
}
