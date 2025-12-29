const std = @import("std");

pub const Error = error{
    GlyphNotFound,
    InvalidFontData,
};

pub const Buf = extern struct {
    data: [*c]u8 = null,
    cursor: c_int = 0,
    size: c_int = 0,
};

pub const FontInfo = extern struct {
    user_data: ?*anyopaque = null,
    data: [*c]u8 = null,
    font_start: c_int = 0,
    num_glyphs: c_int = 0,
    loca: c_int = 0,
    head: c_int = 0,
    glyf: c_int = 0,
    hhea: c_int = 0,
    hmtx: c_int = 0,
    kern: c_int = 0,
    gpos: c_int = 0,
    svg: c_int = 0,
    index_map: c_int = 0,
    index_to_loc_format: c_int = 0,
    cff: Buf = .{},
    charstrings: Buf = .{},
    gsubrs: Buf = .{},
    subrs: Buf = .{},
    fontdicts: Buf = .{},
    fdselect: Buf = .{},
};

pub const BoundingBox = struct {
    x0: i32 = 0,
    y0: i32 = 0,
    x1: i32 = 0,
    y1: i32 = 0,
};

pub const Glyph = struct {
    w: i32 = 0,
    h: i32 = 0,
    xoff: i32 = 0,
    yoff: i32 = 0,
    data: ?[]const u8 = null,
};

pub const GlyphRange = struct {
    items: []Glyph,
    first_unicode_codepoint: u32 = 0,
    num_codepoints: u32 = 0,

    pub fn deinit(self: GlyphRange, allocator: std.mem.Allocator) void {
        for (self.items) |glyph| {
            if (glyph.data) |data| {
                freeSDF(data);
            }
        }
        allocator.free(self.items);
    }
};

pub const VMetrics = struct {
    ascent: i32 = 0,
    descent: i32 = 0,
    line_gap: i32 = 0,
};

pub const HMetrics = struct {
    advance_width: i32,
    left_side_bearing: i32,
};

pub fn initFont(data: []const u8, offset: i32) !FontInfo {
    var info: FontInfo = .{};
    if (stbtt_InitFont(&info, data.ptr, @intCast(offset)) == 0) {
        return Error.InvalidFontData;
    }

    return info;
}

pub fn findGlyphIndex(info: *const FontInfo, unicode_codepoint: u32) !i32 {
    const result = stbtt_FindGlyphIndex(info, @intCast(unicode_codepoint));
    if (result == 0) {
        return Error.GlyphNotFound;
    }

    return @intCast(result);
}

pub fn scaleForPixelHeight(info: *const FontInfo, pixels: f32) f32 {
    return stbtt_ScaleForPixelHeight(info, pixels);
}

pub fn getFontVMetrics(info: *const FontInfo) VMetrics {
    var ascent: c_int = 0;
    var descent: c_int = 0;
    var line_gap: c_int = 0;
    stbtt_GetFontVMetrics(info, &ascent, &descent, &line_gap);
    return .{
        .ascent = @intCast(ascent),
        .descent = @intCast(descent),
        .line_gap = @intCast(line_gap),
    };
}

pub fn getCodepointHMetrics(info: *const FontInfo, codepoint: u32) HMetrics {
    var advance_width: c_int = 0;
    var left_side_bearing: c_int = 0;
    stbtt_GetCodepointHMetrics(info, @intCast(codepoint), &advance_width, &left_side_bearing);
    return .{
        .advance_width = advance_width,
        .left_side_bearing = left_side_bearing,
    };
}

pub fn getGlyphHMetrics(info: *const FontInfo, glyph_index: u32) HMetrics {
    var advance_width: c_int = 0;
    var left_side_bearing: c_int = 0;
    stbtt_GetGlyphHMetrics(info, @intCast(glyph_index), &advance_width, &left_side_bearing);
    return .{
        .advance_width = advance_width,
        .left_side_bearing = left_side_bearing,
    };
}

pub fn getGlyphKernAdvance(info: *const FontInfo, glyph1: i32, glyph2: i32) i32 {
    const result = stbtt_GetGlyphKernAdvance(info, @intCast(glyph1), @intCast(glyph2));
    return @intCast(result);
}

/// Caller must free the data with freeBitmap.
pub fn getCodepointBitmap(info: *const FontInfo, scale_x: f32, scale_y: f32, codepoint: u32) !Glyph {
    var w: c_int = 0;
    var h: c_int = 0;
    var xoff: c_int = 0;
    var yoff: c_int = 0;
    const data = stbtt_GetCodepointBitmap(
        info,
        scale_x,
        scale_y,
        @intCast(codepoint),
        &w,
        &h,
        &xoff,
        &yoff,
    );

    if (data == null) {
        return Error.GlyphNotFound;
    }

    const len: usize = @intCast(w * h);
    return .{
        .w = w,
        .h = h,
        .xoff = xoff,
        .yoff = yoff,
        .data = data[0..len],
    };
}

/// Caller must free the data with freeSDF.
pub fn getCodepointSDF(
    info: *const FontInfo,
    font_size: f32,
    codepoint: u32,
    padding: i32,
    onedge_value: u8,
    pixel_dist_scale: f32,
) !Glyph {
    const scale = stbtt_ScaleForPixelHeight(info, font_size);

    var w: c_int = 0;
    var h: c_int = 0;
    var xoff: c_int = 0;
    var yoff: c_int = 0;

    const bitmap = stbtt_GetCodepointSDF(
        info,
        scale,
        @intCast(codepoint),
        padding, //3,
        onedge_value, //128,
        pixel_dist_scale, //64.0,
        &w,
        &h,
        &xoff,
        &yoff,
    );

    if (bitmap == null) {
        return Error.GlyphNotFound;
    }

    const len: usize = @intCast(w * h);
    return .{
        .w = w,
        .h = h,
        .xoff = xoff,
        .yoff = yoff,
        .data = bitmap[0..len],
    };
}

pub fn getCodepointBitmapBox(
    info: *const FontInfo,
    codepoint: u32,
    scale_x: f32,
    scale_y: f32,
) BoundingBox {
    var x0: c_int = 0;
    var y0: c_int = 0;
    var x1: c_int = 0;
    var y1: c_int = 0;

    stbtt_GetCodepointBitmapBox(
        info,
        @intCast(codepoint),
        scale_x,
        scale_y,
        &x0,
        &y0,
        &x1,
        &y1,
    );

    return .{
        .x0 = @intCast(x0),
        .y0 = @intCast(y0),
        .x1 = @intCast(x1),
        .y1 = @intCast(y1),
    };
}

pub fn getFontBoundingBox(info: *const FontInfo) BoundingBox {
    var x0: c_int = 0;
    var y0: c_int = 0;
    var x1: c_int = 0;
    var y1: c_int = 0;
    stbtt_GetFontBoundingBox(info, &x0, &y0, &x1, &y1);
    return .{
        .x0 = @intCast(x0),
        .y0 = @intCast(y0),
        .x1 = @intCast(x1),
        .y1 = @intCast(y1),
    };
}

pub fn freeBitmap(data: []const u8) void {
    stbtt_FreeBitmap(data.ptr, null);
}

pub fn freeSDF(data: []const u8) void {
    stbtt_FreeSDF(data.ptr, null);
}

extern fn stbtt_InitFont(info: *FontInfo, data: [*c]const u8, offset: c_int) c_int;
extern fn stbtt_FindGlyphIndex(info: *const FontInfo, unicode_codepoint: c_int) c_int;
extern fn stbtt_ScaleForPixelHeight(info: *const FontInfo, pixels: f32) f32;
extern fn stbtt_GetFontVMetrics(
    info: *const FontInfo,
    ascent: *c_int,
    descent: *c_int,
    line_gap: *c_int,
) void;
extern fn stbtt_GetCodepointHMetrics(
    info: *const FontInfo,
    codepoint: c_int,
    advance_width: *c_int,
    left_side_bearing: *c_int,
) void;
extern fn stbtt_GetGlyphHMetrics(
    info: *const FontInfo,
    glyph_index: c_int,
    advance_width: *c_int,
    left_side_bearing: *c_int,
) void;
extern fn stbtt_GetGlyphKernAdvance(info: *const FontInfo, glyph1: c_int, glyph2: c_int) c_int;
extern fn stbtt_GetCodepointBitmap(
    info: *const FontInfo,
    scale_x: f32,
    scale_y: f32,
    codepoint: c_int,
    width: *c_int,
    height: *c_int,
    xoff: *c_int,
    yoff: *c_int,
) [*c]u8;
extern fn stbtt_GetCodepointSDF(
    info: *const FontInfo,
    scale: f32,
    codepoint: c_int,
    padding: c_int,
    onedge_value: u8,
    pixel_dist_scale: f32,
    width: *c_int,
    height: *c_int,
    xoff: *c_int,
    yoff: *c_int,
) [*c]u8;
extern fn stbtt_GetCodepointBitmapBox(
    info: *const FontInfo,
    codepoint: c_int,
    scale_x: f32,
    scale_y: f32,
    ix0: *c_int,
    iy0: *c_int,
    ix1: *c_int,
    iy1: *c_int,
) void;
extern fn stbtt_GetFontBoundingBox(
    info: *const FontInfo,
    x0: *c_int,
    y0: *c_int,
    x1: *c_int,
    y1: *c_int,
) void;
extern fn stbtt_FreeBitmap(bitmap: [*c]const u8, userdata: ?*anyopaque) void;
extern fn stbtt_FreeSDF(bitmap: [*c]const u8, user_data: ?*anyopaque) void;
