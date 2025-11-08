const Atlas = @import("Atlas.zig");
const core = @import("core");
const io = @import("io");
const MemFactory = @import("MemFactory.zig");
const root = @import("root.zig");
const std = @import("std");
const Texture = @import("Texture.zig");
const Textures = @import("Textures.zig");
const TrueType = @import("TrueType");
const Vertex = @import("Vertex.zig");

const Vec2f = core.math.Vec2f;
const VertexBuffer16 = root.VertexBuffer16;

const Self = @This();

pub const Range = struct {
    min: u32,
    max: u32,
};

pub const Glyph = struct {
    min: Vec2f = .ZERO,
    max: Vec2f = .ZERO,
    offset: Vec2f = .ZERO,
    advance_width: i16 = 0,
    left_side_bearing: i16 = 0,

    pub fn width(self: Glyph) f32 {
        return self.max.x - self.min.x;
    }

    pub fn height(self: Glyph) f32 {
        return self.max.y - self.min.y;
    }

    pub fn size(self: Glyph) Vec2f {
        return self.max.sub(self.min);
    }
};
pub const GlyphMap = std.AutoHashMap(u32, Glyph);

path: []const u8,
size: f32,
space_advance_width: i16,
glyphs: GlyphMap,
texture: Texture,
_truetype: TrueType,

pub fn init(
    mem_factory: *MemFactory,
    path: []const u8,
    size: f32,
    textures: *Textures,
) !Self {
    const allocator = mem_factory.allocator;

    const font_path = try io.exeRelativePath(allocator, &.{path});

    // The TrueType object will hold the slice.
    const contents = try io.getContents(allocator, font_path);

    const truetype = try TrueType.load(contents);
    const scale = truetype.scaleForPixelHeight(size);

    var atlas: Atlas = try .init(allocator, 128, 128);
    defer atlas.deinit();

    var pixels = try std.ArrayList(u8).initCapacity(allocator, 0);
    defer pixels.deinit(allocator);

    const range: Range = .{ .min = 0x20, .max = 0xFF };
    var valid_chars = try std.ArrayList(u32).initCapacity(allocator, 0);
    defer valid_chars.deinit(allocator);

    var glyphs = GlyphMap.init(allocator);
    var ch = range.min;
    while (ch <= range.max) {
        defer ch += 1;

        if (truetype.codepointGlyphIndex(@intCast(ch))) |glyph_index| {
            pixels.clearRetainingCapacity();

            const glyph = truetype.glyphBitmap(
                allocator,
                &pixels,
                glyph_index,
                scale,
                scale,
            ) catch |err| {
                switch (err) {
                    TrueType.GlyphBitmapError.GlyphNotFound => continue,
                    else => std.debug.panic(
                        "Failed to get glyph ({}) bitmap: {}",
                        .{ ch, err },
                    ),
                }
            };

            try atlas.place(
                .init(@intCast(glyph.width), @intCast(glyph.height)),
                pixels.items,
            );

            try valid_chars.append(allocator, ch);

            const h_metrics = truetype.glyphHMetrics(glyph_index);
            const entry = try glyphs.getOrPut(ch);
            entry.value_ptr.advance_width = h_metrics.advance_width;
            entry.value_ptr.left_side_bearing = h_metrics.left_side_bearing;
            entry.value_ptr.offset = .init(@floatFromInt(glyph.off_x), @floatFromInt(glyph.off_y));
        }
    }

    // Set up the glyph map after all glyphs have been placed into the atlas.
    for (0..valid_chars.items.len) |i| {
        const char = valid_chars.items[i];
        const region = atlas.regions.items[i];

        const entry = try glyphs.getOrPut(char);
        entry.value_ptr.*.min = .init(
            @floatFromInt(region.min.x),
            @floatFromInt(region.min.y),
        );

        entry.value_ptr.*.max = .init(
            @floatFromInt(region.max.x),
            @floatFromInt(region.max.y),
        );
    }

    const space_advance_width: i16 = if (glyphs.get('i')) |glyph|
        glyph.advance_width
    else
        1;

    var owned = try atlas.buffer.release();
    const texture = try textures.load_buffer(
        mem_factory,
        try owned.data.toOwnedSlice(allocator),
        @intCast(owned.width),
        @intCast(owned.height),
        .grayscale,
    );

    return Self{
        .path = font_path,
        .size = size,
        .space_advance_width = space_advance_width,
        .glyphs = glyphs,
        .texture = texture,
        ._truetype = truetype,
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    allocator.free(self.path);
    allocator.free(self._truetype.ttf_bytes);
    self.glyphs.clearAndFree();
    self.glyphs.deinit();
}

/// Caller must free returned buffer.
pub fn getVertices(
    self: Self,
    allocator: std.mem.Allocator,
    string: []const u8,
    offset: Vec2f,
) !VertexBuffer16 {
    const tex_size: Vec2f = .init(
        @floatFromInt(self.texture.width),
        @floatFromInt(self.texture.height),
    );

    const vertex_count = string.len * 4;
    const index_count = string.len * 6;
    var buffer = try VertexBuffer16.init(allocator, vertex_count, index_count);

    const metrics = self._truetype.verticalMetrics();
    const scale = self._truetype.scaleForPixelHeight(self.size);
    const ascent: f32 = @as(f32, @floatFromInt(metrics.ascent)) * scale;

    var v_index: usize = 0;
    var vertex_offset: u16 = 0;
    var index: usize = 0;
    var cursor: Vec2f = offset;
    var prev_ch: u32 = 0;
    for (string) |ch| {
        if (ch == ' ') {
            cursor.x += @as(f32, @floatFromInt(self.space_advance_width)) * scale;
            prev_ch = 0;
            continue;
        }

        if (self.glyphs.get(ch)) |glyph| {
            // The offset has already been scaled when rendering the bitmap. No need to apply here.
            const glyph_offset: Vec2f = glyph.offset;
            const left_side_bearing: f32 = @as(f32, @floatFromInt(glyph.left_side_bearing)) * scale;

            var min: Vec2f = cursor;
            if (self.getKernAdvance(prev_ch, ch)) |advance| {
                min.x += @as(f32, @floatFromInt(advance)) * scale;
            }

            min.x += left_side_bearing + glyph_offset.x;
            min.y += ascent + glyph_offset.y;

            var max: Vec2f = min.add(glyph.size());

            min.x = @floor(min.x);
            min.y = @floor(min.y);
            max.x = @floor(max.x);
            max.y = @floor(max.y);

            _ = buffer.vertices[v_index + 0].setPositionVec2(min);
            _ = buffer.vertices[v_index + 1].setPositionVec2(.init(min.x, max.y));
            _ = buffer.vertices[v_index + 2].setPositionVec2(max);
            _ = buffer.vertices[v_index + 3].setPositionVec2(.init(max.x, min.y));

            const uv_min = glyph.min.div(tex_size);
            const uv_max = glyph.max.div(tex_size);

            _ = buffer.vertices[v_index + 0].setUVVec2(uv_min);
            _ = buffer.vertices[v_index + 1].setUVVec2(.init(uv_min.x, uv_max.y));
            _ = buffer.vertices[v_index + 2].setUVVec2(uv_max);
            _ = buffer.vertices[v_index + 3].setUVVec2(.init(uv_max.x, uv_min.y));

            _ = buffer.vertices[v_index + 0].setColor(255, 255, 255, 255);
            _ = buffer.vertices[v_index + 1].setColor(255, 255, 255, 255);
            _ = buffer.vertices[v_index + 2].setColor(255, 255, 255, 255);
            _ = buffer.vertices[v_index + 3].setColor(255, 255, 255, 255);

            buffer.indices[index + 0] = vertex_offset + 0;
            buffer.indices[index + 1] = vertex_offset + 1;
            buffer.indices[index + 2] = vertex_offset + 2;
            buffer.indices[index + 3] = vertex_offset + 0;
            buffer.indices[index + 4] = vertex_offset + 2;
            buffer.indices[index + 5] = vertex_offset + 3;

            v_index += 4;
            vertex_offset += 4;
            index += 6;

            cursor.x += @as(f32, @floatFromInt(glyph.advance_width)) * scale;
            prev_ch = ch;
        }
    }

    return buffer;
}

fn getKernAdvance(self: Self, a: u32, b: u32) ?i16 {
    const glyph_a = self._truetype.codepointGlyphIndex(@intCast(a));
    const glyph_b = self._truetype.codepointGlyphIndex(@intCast(b));

    if (glyph_a == null or glyph_b == null) {
        return null;
    }

    return self._truetype.glyphKernAdvance(glyph_a.?, glyph_b.?);
}
