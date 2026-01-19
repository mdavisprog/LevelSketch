const core = @import("core");
const io = @import("io");
const render = @import("root.zig");
const stb = @import("stb");
const std = @import("std");

const Vec2f = core.math.Vec2f;

const Atlas = render.Atlas;
const MemFactory = render.MemFactory;
const Renderer = render.Renderer;
const Texture = render.Texture;
const Textures = render.Textures;
const Vertex = render.Vertex;
const UIVertexBuffer16 = render.UIVertexBuffer16;

const FontInfo = stb.truetype.FontInfo;

const Self = @This();

pub const Handle = render.Handle(Self);

pub const Range = struct {
    min: u32,
    max: u32,
};

pub const Glyph = struct {
    min: Vec2f = .zero,
    max: Vec2f = .zero,
    offset: Vec2f = .zero,
    // These are already scaled.
    advance_width: f32 = 0.0,
    left_side_bearing: f32 = 0.0,

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

pub const VMetrics = struct {
    scale: f32 = 0.0,
    ascent: f32 = 0.0,
    descent: f32 = 0.0,
    line_gap: f32 = 0.0,
};

pub const Type = enum {
    bitmap,
    sdf,
};

handle: Handle = .invalid,
size: f32 = 0.0,
scale: f32 = 1.0,
space_advance_width: f32 = 0.0,
v_metrics: VMetrics = .{},
glyphs: GlyphMap,
texture: Texture = .{},
_info: FontInfo,
_data: []const u8,

/// 'path' must be the full path. Creates the stb_truetype font info object and sets up this
/// object for loading glyphs.
pub fn init(
    allocator: std.mem.Allocator,
    path: []const u8,
) !*Self {
    const contents = try io.getContents(allocator, path);
    const info = try stb.truetype.initFont(contents, 0);

    const result = try allocator.create(Self);
    result.* = .{
        .glyphs = .init(allocator),
        ._info = info,
        ._data = contents,
    };

    return result;
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    allocator.free(self._data);
    self.glyphs.clearAndFree();
    self.glyphs.deinit();
}

pub fn load(
    self: *Self,
    renderer: *Renderer,
    size: f32,
    font_type: Type,
) !void {
    const allocator = renderer.mem_factory.allocator;

    const scale = stb.truetype.scaleForPixelHeight(&self._info, size);

    var atlas: Atlas = try .init(allocator, 128, 128);
    defer atlas.deinit();

    const range: Range = .{ .min = 0x20, .max = 0xFF };
    var valid_chars = try std.ArrayList(u32).initCapacity(allocator, 0);
    defer valid_chars.deinit(allocator);

    var ch = range.min;
    while (ch <= range.max) {
        defer ch += 1;

        const glyph_or_error = switch (font_type) {
            .bitmap => stb.truetype.getCodepointBitmap(&self._info, scale, scale, ch),
            .sdf => stb.truetype.getCodepointSDF(&self._info, size, ch, 4, 128, 32.0),
        };

        const glyph = glyph_or_error catch |err| {
            switch (err) {
                stb.truetype.Error.GlyphNotFound => {
                    if (ch == ' ') {
                        const metrics = stb.truetype.getCodepointHMetrics(&self._info, ch);
                        self.space_advance_width =
                            @as(f32, @floatFromInt(metrics.advance_width)) * scale;
                    }
                    continue;
                },
                else => std.debug.panic(
                    "Failed to get bitmap for codepoint {} '{c}'. Error: {}",
                    .{
                        ch,
                        @as(u8, @intCast(ch)),
                        err,
                    },
                ),
            }
        };

        const glyph_data = glyph.data orelse continue;
        defer {
            switch (font_type) {
                .bitmap => stb.truetype.freeBitmap(glyph_data),
                .sdf => stb.truetype.freeSDF(glyph_data),
            }
        }

        try atlas.place(
            .init(@intCast(glyph.w), @intCast(glyph.h)),
            glyph_data,
        );

        try valid_chars.append(allocator, ch);

        const h_metrics = stb.truetype.getCodepointHMetrics(&self._info, ch);
        const entry = try self.glyphs.getOrPut(ch);
        entry.value_ptr.advance_width =
            @as(f32, @floatFromInt(h_metrics.advance_width)) * scale;
        entry.value_ptr.left_side_bearing =
            @as(f32, @floatFromInt(h_metrics.left_side_bearing)) * scale;
        entry.value_ptr.offset = .init(@floatFromInt(glyph.xoff), @floatFromInt(glyph.yoff));
    }

    // Set up the glyph map after all glyphs have been placed into the atlas.
    for (0..valid_chars.items.len) |i| {
        const char = valid_chars.items[i];
        const region = atlas.regions.items[i];

        const entry = try self.glyphs.getOrPut(char);
        entry.value_ptr.*.min = .init(
            @floatFromInt(region.min.x),
            @floatFromInt(region.min.y),
        );

        entry.value_ptr.*.max = .init(
            @floatFromInt(region.max.x),
            @floatFromInt(region.max.y),
        );
    }

    const metrics = stb.truetype.getFontVMetrics(&self._info);
    self.v_metrics.scale = scale;
    self.v_metrics.ascent = @as(f32, @floatFromInt(metrics.ascent)) * scale;
    self.v_metrics.descent = @as(f32, @floatFromInt(metrics.descent)) * scale;
    self.v_metrics.line_gap = @as(f32, @floatFromInt(metrics.line_gap)) * scale;

    self.size = size;
    self.scale = 1.0;

    var owned = try atlas.buffer.release();
    self.texture = try renderer.textures.loadBuffer(
        &renderer.mem_factory,
        try owned.data.toOwnedSlice(allocator),
        @intCast(owned.width),
        @intCast(owned.height),
        .grayscale,
    );
}

/// Caller must free returned buffer.
pub fn getVertices(
    self: Self,
    allocator: std.mem.Allocator,
    string: []const u8,
    offset: Vec2f,
) !UIVertexBuffer16 {
    const tex_size: Vec2f = .init(
        @floatFromInt(self.texture.width),
        @floatFromInt(self.texture.height),
    );

    const vertex_count = string.len * 4;
    const index_count = string.len * 6;
    var buffer = try UIVertexBuffer16.init(allocator, vertex_count, index_count);

    var v_index: usize = 0;
    var vertex_offset: u16 = 0;
    var index: usize = 0;
    var cursor: Vec2f = offset;
    var prev_ch: u32 = 0;
    for (string) |ch| {
        if (ch == ' ') {
            cursor.x += self.space_advance_width * self.scale;
            prev_ch = 0;
            continue;
        }

        if (self.glyphs.get(ch)) |glyph| {
            const glyph_offset: Vec2f = glyph.offset.mulScalar(self.scale);
            const glyph_size: Vec2f = glyph.size().mulScalar(self.scale);
            const kern = self.getKernAdvance(prev_ch, ch);

            var min: Vec2f = cursor;
            min.x += @as(f32, @floatFromInt(kern)) * self.v_metrics.scale;
            min.x += glyph.left_side_bearing * self.scale + glyph_offset.x;
            min.y += glyph_offset.y + self.v_metrics.ascent * self.scale;

            var max: Vec2f = min.add(glyph_size);

            min.x = @floor(min.x);
            min.y = @floor(min.y);
            max.x = @floor(max.x);
            max.y = @floor(max.y);

            _ = buffer.vertices.items[v_index + 0].setPosition(min.toVec());
            _ = buffer.vertices.items[v_index + 1].setPosition(.init2(min.x, max.y));
            _ = buffer.vertices.items[v_index + 2].setPosition(max.toVec());
            _ = buffer.vertices.items[v_index + 3].setPosition(.init2(max.x, min.y));

            const uv_min = glyph.min.div(tex_size);
            const uv_max = glyph.max.div(tex_size);

            _ = buffer.vertices.items[v_index + 0].setUV(uv_min);
            _ = buffer.vertices.items[v_index + 1].setUV(.init(uv_min.x, uv_max.y));
            _ = buffer.vertices.items[v_index + 2].setUV(uv_max);
            _ = buffer.vertices.items[v_index + 3].setUV(.init(uv_max.x, uv_min.y));

            buffer.vertices.items[v_index + 0].abgr = 0xFFFFFFFF;
            buffer.vertices.items[v_index + 1].abgr = 0xFFFFFFFF;
            buffer.vertices.items[v_index + 2].abgr = 0xFFFFFFFF;
            buffer.vertices.items[v_index + 3].abgr = 0xFFFFFFFF;

            buffer.indices.items[index + 0] = vertex_offset + 0;
            buffer.indices.items[index + 1] = vertex_offset + 1;
            buffer.indices.items[index + 2] = vertex_offset + 2;
            buffer.indices.items[index + 3] = vertex_offset + 0;
            buffer.indices.items[index + 4] = vertex_offset + 2;
            buffer.indices.items[index + 5] = vertex_offset + 3;

            v_index += 4;
            vertex_offset += 4;
            index += 6;

            cursor.x += glyph.advance_width * self.scale;
            prev_ch = ch;
        }
    }

    return buffer;
}

pub fn measure(self: Self, text: []const u8) Vec2f {
    var result: Vec2f = .zero;
    var prev_ch: u32 = 0;
    for (text) |ch| {
        if (ch == ' ') {
            result.x += self.space_advance_width * self.scale;
            prev_ch = 0;
            continue;
        }

        if (self.glyphs.get(ch)) |glyph| {
            const glyph_size = glyph.size().mulScalar(self.scale);

            result.x += glyph.advance_width * self.scale;
            result.y = @max(result.y, glyph_size.y);
        }
    }

    return result;
}

pub fn scaleToSize(self: *Self, size: f32) void {
    self.scale = size / self.size;
}

fn getKernAdvance(self: Self, a: u32, b: u32) i32 {
    const glyph_a = stb.truetype.findGlyphIndex(&self._info, a) catch return 0;
    const glyph_b = stb.truetype.findGlyphIndex(&self._info, b) catch return 0;
    return stb.truetype.getGlyphKernAdvance(&self._info, glyph_a, glyph_b);
}
