const rect_pack = @cImport({
    @cInclude("stb_rect_pack.h");
});
const std = @import("std");
const truetype = @cImport({
    @cInclude("stb_truetype.h");
});

pub const Error = error{
    LoadFailed,
};

pub const Range = struct {
    min: u32,
    max: u32,
};

pub const LoadResult = struct {
    glyphs: std.AutoHashMap(u32, truetype.stbtt_packedchar),
    texture: []u8,
    ascent: f32,
    descent: f32,

    pub fn deinit(self: *LoadResult, allocator: std.mem.Allocator) void {
        self.glyphs.deinit();
        allocator.free(self.texture);
    }
};

pub fn load(allocator: std.mem.Allocator, data: []const u8, font_size: f32) !LoadResult {
    var ascent: f32 = 0.0;
    var descent: f32 = 0.0;
    var line_gap: f32 = 0.0;
    truetype.stbtt_GetScaledFontVMetrics(
        data.ptr,
        0,
        font_size,
        &ascent,
        &descent,
        &line_gap,
    );

    var texture_size: [2]f32 = .{ 128.0, 128.0 };
    const font_offset = truetype.stbtt_GetFontOffsetForIndex(data.ptr, 0);

    var font_info: truetype.stbtt_fontinfo = .{};
    _ = truetype.stbtt_InitFont(&font_info, data.ptr, font_offset);

    var ctx: truetype.stbtt_pack_context = .{};
    _ = truetype.stbtt_PackBegin(
        &ctx,
        null,
        @intFromFloat(texture_size[0]),
        @intFromFloat(texture_size[1]),
        0,
        1,
        null,
    );

    // Ranges determine what code points to use.
    // TODO: Specify ranges in argument
    const ranges: [1]Range = .{
        .{ .min = 0x20, .max = 0x7F },
    };

    var chars = try std.ArrayList(truetype.stbtt_packedchar).initCapacity(allocator, 0);
    var all_rects = try std.ArrayList(rect_pack.stbrp_rect).initCapacity(allocator, 0);
    var pack_ranges = try std.ArrayList(truetype.stbtt_pack_range).initCapacity(allocator, 0);

    defer chars.deinit(allocator);
    defer all_rects.deinit(allocator);
    defer pack_ranges.deinit(allocator);

    for (ranges) |range| {
        const pack_range = try pack_ranges.addOne(allocator);
        pack_range.*.font_size = font_size;
        pack_range.*.first_unicode_codepoint_in_range = @intCast(range.min);
        pack_range.num_chars = @intCast(range.max - range.min + 1);
        pack_range.array_of_unicode_codepoints = null;

        const num_chars: usize = @intCast(pack_range.num_chars);
        try chars.appendNTimes(allocator, .{}, num_chars);

        var rects = try std.ArrayList(rect_pack.stbrp_rect).initCapacity(allocator, num_chars);
        defer rects.deinit(allocator);
        try rects.appendNTimes(allocator, .{}, num_chars);

        var list = [_]truetype.stbtt_pack_range{pack_range.*};
        _ = packFontRangesGatherRects(
            ctx,
            font_info,
            &list,
            rects.items,
        );

        try all_rects.appendSlice(allocator, rects.items);

        truetype.stbtt_PackSetSkipMissingCodepoints(&ctx, 1);
    }

    while (true) {
        const width: i32 = @intFromFloat(texture_size[0]);
        const height: i32 = @intFromFloat(texture_size[1]);
        const padding: i32 = @intCast(ctx.padding);

        var context: rect_pack.stbrp_context = .{};

        const capacity: usize = @intCast(width - padding);
        var nodes = try std.ArrayList(rect_pack.stbrp_node).initCapacity(allocator, capacity);
        defer nodes.deinit(allocator);

        try nodes.appendNTimes(allocator, .{}, capacity);
        rect_pack.stbrp_init_target(
            &context,
            width - padding,
            height - padding,
            nodes.items.ptr,
            @intCast(nodes.items.len),
        );

        const result = rect_pack.stbrp_pack_rects(
            &context,
            all_rects.items.ptr,
            @intCast(all_rects.items.len),
        );

        if (result == 1) {
            break;
        } else {
            // Increase the texture size and try again.
            if (texture_size[1] < texture_size[0]) {
                texture_size[1] = texture_size[0];
            } else {
                texture_size[0] += 128.0;
            }
        }
    }

    // Reset to not skip when rendering each rect.
    truetype.stbtt_PackSetSkipMissingCodepoints(&ctx, 0);

    const texture_data_len: usize = @as(usize, @intFromFloat(texture_size[0])) *
        @as(usize, @intFromFloat(texture_size[1]));
    const texture = try allocator.alloc(u8, texture_data_len);

    @memset(texture, 0);

    ctx.width = @intFromFloat(texture_size[0]);
    ctx.height = @intFromFloat(texture_size[1]);
    ctx.pixels = texture.ptr;
    ctx.stride_in_bytes = ctx.width;

    // Need to update the pointers for chardata_for_range
    var offset: usize = 0;
    for (pack_ranges.items) |*range| {
        range.chardata_for_range = &chars.items[offset];
        offset += @intCast(range.num_chars);
    }

    const success = packFontRangesRenderIntoRects(
        &ctx,
        font_info,
        pack_ranges.items,
        all_rects.items,
    );

    truetype.stbtt_PackEnd(&ctx);

    if (!success) {
        return Error.LoadFailed;
    }

    var index: usize = 0;
    var glyphs = std.AutoHashMap(u32, truetype.stbtt_packedchar).init(allocator);
    for (ranges) |range| {
        var codepoint = range.min;
        while (codepoint <= range.max) {
            const char = chars.items[@intCast(index)];
            index += 1;

            try glyphs.put(codepoint, char);
            codepoint += 1;
        }
    }

    return LoadResult{
        .glyphs = glyphs,
        .texture = texture,
        .ascent = ascent,
        .descent = descent,
    };
}

///
/// Below are functions that have been re-implemented in Zig. There were issues with C struct
/// field ordering when the C function accepts a raw C pointer as an argument. Only functions
/// that accepts this type of pointer as an argument have been rewritten.
///
fn packFontRangesGatherRects(
    context: truetype.stbtt_pack_context,
    info: truetype.stbtt_fontinfo,
    ranges: []truetype.stbtt_pack_range,
    rects: []rect_pack.stbrp_rect,
) bool {
    for (0..ranges.len) |i| {
        const fh = ranges[i].font_size;
        const scale = if (fh > 0.0)
            truetype.stbtt_ScaleForPixelHeight(&info, fh)
        else
            truetype.stbtt_ScaleForMappingEmToPixels(&info, -fh);

        ranges[i].h_oversample = @intCast(context.h_oversample);
        ranges[i].v_oversample = @intCast(context.v_oversample);

        var k: usize = 0;
        var missing_glyph_added = false;
        const num_chars: usize = @intCast(ranges[i].num_chars);
        for (0..num_chars) |j| {
            const codepoint = if (ranges[i].array_of_unicode_codepoints == null)
                ranges[i].first_unicode_codepoint_in_range + @as(c_int, @intCast(j))
            else
                ranges[i].array_of_unicode_codepoints[j];

            const glyph = truetype.stbtt_FindGlyphIndex(&info, codepoint);
            if (glyph == 0 and (context.skip_missing == 1 or missing_glyph_added)) {
                rects[k].w = 0;
                rects[k].h = 0;
            } else {
                var x0: c_int = 0;
                var y0: c_int = 0;
                var x1: c_int = 0;
                var y1: c_int = 0;
                truetype.stbtt_GetGlyphBitmapBoxSubpixel(
                    &info,
                    glyph,
                    scale * @as(f32, @floatFromInt(context.h_oversample)),
                    scale * @as(f32, @floatFromInt(context.v_oversample)),
                    0,
                    0,
                    &x0,
                    &y0,
                    &x1,
                    &y1,
                );

                const h_oversample: c_int = @intCast(context.h_oversample);
                const v_oversample: c_int = @intCast(context.v_oversample);
                rects[k].w = x1 - x0 + context.padding + h_oversample - 1;
                rects[k].h = y1 - y0 + context.padding + v_oversample - 1;

                if (glyph == 0) {
                    missing_glyph_added = true;
                }
            }

            k += 1;
        }
    }

    return false;
}

fn packFontRangesRenderIntoRects(
    context: *truetype.stbtt_pack_context,
    info: truetype.stbtt_fontinfo,
    ranges: []truetype.stbtt_pack_range,
    rects: []rect_pack.stbrp_rect,
) bool {
    var result = true;

    var missing_glyph: i32 = -1;
    const old_h_over = context.h_oversample;
    const old_v_over = context.v_oversample;

    var k: usize = 0;
    for (0..ranges.len) |i| {
        const fh = ranges[i].font_size;
        const scale = if (fh > 0.0)
            truetype.stbtt_ScaleForPixelHeight(&info, fh)
        else
            truetype.stbtt_ScaleForMappingEmToPixels(&info, -fh);

        context.*.h_oversample = ranges[i].h_oversample;
        context.*.v_oversample = ranges[i].v_oversample;

        const recip_h = 1.0 / @as(f32, @floatFromInt(context.h_oversample));
        const recip_v = 1.0 / @as(f32, @floatFromInt(context.v_oversample));

        const sub_x = oversampleShift(context.h_oversample);
        const sub_y = oversampleShift(context.v_oversample);

        const num_chars: usize = @intCast(ranges[i].num_chars);
        for (0..num_chars) |j| {
            var r = &rects[k];
            if (r.was_packed == 1 and r.w != 0 and r.h != 0) {
                const bc = &ranges[i].chardata_for_range[j];
                const codepoint = if (ranges[i].array_of_unicode_codepoints == null)
                    ranges[i].first_unicode_codepoint_in_range + @as(c_int, @intCast(j))
                else
                    ranges[i].array_of_unicode_codepoints[j];
                const glyph = truetype.stbtt_FindGlyphIndex(&info, codepoint);
                const pad = context.padding;

                r.x += pad;
                r.y += pad;
                r.w -= pad;
                r.h -= pad;

                var advance: c_int = 0;
                var lsb: c_int = 0;
                truetype.stbtt_GetGlyphHMetrics(&info, glyph, &advance, &lsb);

                var x0: c_int = 0;
                var y0: c_int = 0;
                var x1: c_int = 0;
                var y1: c_int = 0;
                truetype.stbtt_GetGlyphBitmapBox(
                    &info,
                    glyph,
                    scale * @as(f32, @floatFromInt(context.h_oversample)),
                    scale * @as(f32, @floatFromInt(context.v_oversample)),
                    &x0,
                    &y0,
                    &x1,
                    &y1,
                );

                const offset: usize = @intCast(r.x + r.y * context.stride_in_bytes);
                truetype.stbtt_MakeGlyphBitmapSubpixel(
                    &info,
                    context.pixels + offset,
                    r.w - @as(c_int, @intCast(context.h_oversample + 1)),
                    r.h - @as(c_int, @intCast(context.v_oversample + 1)),
                    context.stride_in_bytes,
                    scale * @as(f32, @floatFromInt(context.h_oversample)),
                    scale * @as(f32, @floatFromInt(context.v_oversample)),
                    0,
                    0,
                    glyph,
                );

                if (context.h_oversample > 1) {
                    prefilterH(
                        context.pixels + offset,
                        r.w,
                        r.h,
                        context.stride_in_bytes,
                        context.h_oversample,
                    );
                }

                if (context.v_oversample > 1) {
                    prefilterV(
                        context.pixels + offset,
                        r.w,
                        r.h,
                        context.stride_in_bytes,
                        context.v_oversample,
                    );
                }

                bc.*.x0 = @intCast(r.x);
                bc.*.y0 = @intCast(r.y);
                bc.*.x1 = @intCast(r.x + r.w);
                bc.*.y1 = @intCast(r.y + r.h);
                bc.*.xadvance = scale * @as(f32, @floatFromInt(advance));
                bc.*.xoff = @as(f32, @floatFromInt(x0)) * recip_h + sub_x;
                bc.*.yoff = @as(f32, @floatFromInt(y0)) * recip_v + sub_y;
                bc.*.xoff2 = @as(f32, @floatFromInt(x0 + r.w)) * recip_h + sub_x;
                bc.*.yoff2 = @as(f32, @floatFromInt(y0 + r.h)) * recip_v + sub_y;

                if (glyph == 0) {
                    missing_glyph = @intCast(j);
                }
            } else if (context.skip_missing == 1) {
                result = false;
            } else if (r.was_packed == 1 and r.w == 0 and r.h == 0 and missing_glyph >= 0) {
                ranges[i].chardata_for_range[j] =
                    ranges[i].chardata_for_range[@intCast(missing_glyph)];
            } else {
                result = false;
            }

            k += 1;
        }
    }

    context.*.h_oversample = old_h_over;
    context.*.v_oversample = old_v_over;

    return result;
}

fn oversampleShift(oversample: c_uint) f32 {
    if (oversample == 0) {
        return 0.0;
    }

    return @as(f32, @floatFromInt(oversample - 1)) /
        (2.0 * @as(f32, @floatFromInt(oversample)));
}

const STBTT_MAX_OVERSAMPLE: usize = 8;
const STBTT__OVER_MASK = STBTT_MAX_OVERSAMPLE - 1;

fn prefilterH(
    pixels: [*c]u8,
    w: c_int,
    h: c_int,
    stride_in_bytes: c_int,
    kernel_width: c_uint,
) void {
    var buffer: [STBTT_MAX_OVERSAMPLE]u8 = .{0} ** STBTT_MAX_OVERSAMPLE;
    const safe_w = w - @as(c_int, @intCast(kernel_width));

    for (0..@intCast(h)) |_| {
        @memset(&buffer, 0);

        var total: c_uint = 0;
        var i: usize = 0;
        switch (kernel_width) {
            2 => {
                while (i <= safe_w) {
                    total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                    pixels[i] = @intCast(total / 2);
                    i += 1;
                }
            },
            3 => {
                while (i <= safe_w) {
                    total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                    pixels[i] = @intCast(total / 3);
                    i += 1;
                }
            },
            4 => {
                while (i <= safe_w) {
                    total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                    pixels[i] = @intCast(total / 4);
                    i += 1;
                }
            },
            5 => {
                while (i <= safe_w) {
                    total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                    pixels[i] = @intCast(total / 5);
                    i += 1;
                }
            },
            else => {
                while (i <= safe_w) {
                    total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                    pixels[i] = @intCast(total / kernel_width);
                    i += 1;
                }
            },
        }

        while (i < w) {
            //std.debug.assert(pixels[i] == 0);
            total -= buffer[i & STBTT__OVER_MASK];
            pixels[i] = @intCast(total / kernel_width);
            i += 1;
        }

        pixels.* += @intCast(stride_in_bytes);
    }
}

fn prefilterV(
    pixels: [*c]u8,
    w: c_int,
    h: c_int,
    stride_in_bytes: c_int,
    kernel_width: c_uint,
) void {
    var buffer: [STBTT_MAX_OVERSAMPLE]u8 = .{0} ** STBTT_MAX_OVERSAMPLE;
    const safe_h = h - @as(c_int, @intCast(kernel_width));

    for (0..@intCast(w)) |_| {
        @memset(&buffer, 0);

        const stride: usize = @intCast(stride_in_bytes);
        var total: c_uint = 0;
        var i: usize = 0;
        switch (kernel_width) {
            2 => {
                while (i <= safe_h) {
                    total += pixels[i * stride] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride];
                    pixels[i * stride] = @intCast(total / 2);
                    i += 1;
                }
            },
            3 => {
                while (i <= safe_h) {
                    total += pixels[i * stride] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride];
                    pixels[i * stride] = @intCast(total / 3);
                    i += 1;
                }
            },
            4 => {
                while (i <= safe_h) {
                    total += pixels[i * stride] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride];
                    pixels[i * stride] = @intCast(total / 4);
                    i += 1;
                }
            },
            5 => {
                while (i <= safe_h) {
                    total += pixels[i * stride] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride];
                    pixels[i * stride] = @intCast(total / 5);
                    i += 1;
                }
            },
            else => {
                while (i <= safe_h) {
                    total += pixels[i * stride] - buffer[i & STBTT__OVER_MASK];
                    buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride];
                    pixels[i * stride] = @intCast(total / kernel_width);
                    i += 1;
                }
            },
        }

        while (i < h) {
            //std.debug.assert(pixels[i * stride] == 0);
            total -= buffer[i & STBTT__OVER_MASK];
            pixels[i * stride] = @intCast(total / kernel_width);
            i += 1;
        }

        pixels.* += 1;
    }
}
