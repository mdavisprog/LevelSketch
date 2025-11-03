const std = @import("std");
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const expectEqualSlices = std.testing.expectEqualSlices;

const TrueType = @import("TrueType");
const ttf_data = @embedFile("GoNotoCurrent-Regular.ttf");
const c = @import("c");

const max_codepoint = 0x10FFFF;

test "glyph index lookup" {
    const ttf = try TrueType.load(ttf_data);

    var stb_font: c.stbtt_fontinfo = undefined;
    try expect(c.stbtt_InitFont(&stb_font, ttf_data, 0) != 0);

    try expectEqualInts(stb_font.loca, ttf.table_offsets[@intFromEnum(TrueType.TableId.loca)]);
    try expectEqualInts(stb_font.head, ttf.table_offsets[@intFromEnum(TrueType.TableId.head)]);
    try expectEqualInts(stb_font.glyf, ttf.table_offsets[@intFromEnum(TrueType.TableId.glyf)]);
    try expectEqualInts(stb_font.hhea, ttf.table_offsets[@intFromEnum(TrueType.TableId.hhea)]);
    try expectEqualInts(stb_font.hmtx, ttf.table_offsets[@intFromEnum(TrueType.TableId.hmtx)]);
    try expectEqualInts(stb_font.kern, ttf.table_offsets[@intFromEnum(TrueType.TableId.kern)]);
    try expectEqualInts(stb_font.gpos, ttf.table_offsets[@intFromEnum(TrueType.TableId.GPOS)]);

    {
        var stb_ascent: c_int = undefined;
        var stb_descent: c_int = undefined;
        var stb_line_gap: c_int = undefined;
        c.stbtt_GetFontVMetrics(&stb_font, &stb_ascent, &stb_descent, &stb_line_gap);
        const vm = ttf.verticalMetrics();
        try expectEqualInts(stb_ascent, vm.ascent);
        try expectEqualInts(stb_descent, vm.descent);
        try expectEqualInts(stb_line_gap, vm.line_gap);
    }

    try expectEqualInts(stb_font.numGlyphs, ttf.glyphs_len);
    try expectEqualInts(stb_font.indexToLocFormat, ttf.index_to_loc_format);
    try expectEqualInts(stb_font.index_map, ttf.index_map);

    for (0..max_codepoint) |codepoint| {
        const zig_answer = if (ttf.codepointGlyphIndex(@intCast(codepoint))) |x| @intFromEnum(x) else 0;
        const stb_answer = c.stbtt_FindGlyphIndex(&stb_font, @intCast(codepoint));
        try expectEqualInts(stb_answer, zig_answer);
    }
}

test "glyph h metrics" {
    const ttf = try TrueType.load(ttf_data);
    var stb_font: c.stbtt_fontinfo = undefined;
    try expect(c.stbtt_InitFont(&stb_font, ttf_data, 0) != 0);
    try expectEqualInts(stb_font.numGlyphs, ttf.glyphs_len);

    for (0..ttf.glyphs_len) |glyph_index| {
        //std.debug.print("glyph_index={d}/{d}\n", .{ glyph_index, ttf.glyphs_len });
        const zig_answer = ttf.glyphHMetrics(@enumFromInt(glyph_index));

        var stb_advance_width: c_int = undefined;
        var stb_left_side_bearing: c_int = undefined;
        c.stbtt_GetGlyphHMetrics(&stb_font, @intCast(glyph_index), &stb_advance_width, &stb_left_side_bearing);

        try expectEqual(stb_advance_width, zig_answer.advance_width);
        try expectEqual(stb_left_side_bearing, zig_answer.left_side_bearing);
    }
}

test "glyph kern advance" {
    const ttf = try TrueType.load(ttf_data);
    var stb_font: c.stbtt_fontinfo = undefined;
    try expect(c.stbtt_InitFont(&stb_font, ttf_data, 0) != 0);
    try expectEqualInts(stb_font.numGlyphs, ttf.glyphs_len);

    // I tested this with every combination of pairs once and it fully passed,
    // but that takes 2 days to run so let's just test a sample instead.

    var rng_instance: std.Random.DefaultPrng = .init(std.testing.random_seed);
    const rng = rng_instance.random();

    for (0..500_000) |_| {
        const a = rng.uintLessThan(u32, ttf.glyphs_len);
        const b = rng.uintLessThan(u32, ttf.glyphs_len);
        //if (b == 0) std.debug.print("glyph_index={d}/{d}\n", .{ a * ttf.glyphs_len + b, ttf.glyphs_len * ttf.glyphs_len });
        const stb_answer = c.stbtt_GetGlyphKernAdvance(&stb_font, @intCast(a), @intCast(b));
        const zig_answer = ttf.glyphKernAdvance(@enumFromInt(a), @enumFromInt(b));
        try expectEqual(stb_answer, zig_answer);
    }
}

test "glyph bitmap rendering" {
    const gpa = std.testing.allocator;

    const ttf = try TrueType.load(ttf_data);
    var stb_font: c.stbtt_fontinfo = undefined;
    try expect(c.stbtt_InitFont(&stb_font, ttf_data, 0) != 0);
    try expectEqualInts(stb_font.numGlyphs, ttf.glyphs_len);

    var buffer: std.ArrayListUnmanaged(u8) = .empty;
    defer buffer.deinit(gpa);

    const scale = ttf.scaleForPixelHeight(32);
    try expectEqual(c.stbtt_ScaleForPixelHeight(&stb_font, 32), scale);

    for (0..ttf.glyphs_len) |glyph_index| {
        buffer.clearRetainingCapacity();

        //std.debug.print("glyph_index={d}/{d}\n", .{ glyph_index, ttf.glyphs_len });

        var stb_width: c_int = undefined;
        var stb_height: c_int = undefined;
        var stb_xoff: c_int = undefined;
        var stb_yoff: c_int = undefined;
        const stb_pixels = c.stbtt_GetGlyphBitmap(&stb_font, scale, scale, @intCast(glyph_index), &stb_width, &stb_height, &stb_xoff, &stb_yoff);
        defer c.stbtt_FreeBitmap(stb_pixels, null);

        const dims = ttf.glyphBitmap(gpa, &buffer, @enumFromInt(glyph_index), scale, scale) catch |err| switch (err) {
            error.GlyphNotFound => {
                try expect(stb_pixels == null);
                continue;
            },
            error.OutOfMemory => return error.OutOfMemory,
        };

        try expectEqualInts(stb_width, dims.width);
        try expectEqualInts(stb_height, dims.height);
        try expectEqualInts(stb_xoff, dims.off_x);
        try expectEqualInts(stb_yoff, dims.off_y);
        try expectEqual(buffer.items.len, dims.width * dims.height);
        // 55 of the glyphs have some bytes off by exactly 1.
        try expectNearlyEqual(stb_pixels[0..buffer.items.len], buffer.items);
    }
}

fn expectEqualInts(expected: anytype, actual: anytype) anyerror!void {
    const actual_casted = std.math.cast(@TypeOf(expected), actual) orelse {
        std.debug.print("expected {any}, found {any}\n", .{ expected, actual });
        return error.TestFailed;
    };
    try std.testing.expectEqual(expected, actual_casted);
}

fn expectNearlyEqual(expected: []const u8, actual: []const u8) anyerror!void {
    if (expected.len != actual.len) return expectEqualSlices(u8, expected, actual);
    if (std.mem.eql(u8, expected, actual)) return;

    var max_dist: usize = 0;
    var total_dist: usize = 0;
    for (expected, actual) |e, a| {
        const dist = @max(e, a) - @min(e, a);
        max_dist = @max(dist, max_dist);
        total_dist += 1;
    }
    if (total_dist > 1500 or max_dist > 1) {
        std.debug.print("total_dist={d} max_dist={d}\n", .{ total_dist, max_dist });
        return expectEqualSlices(u8, expected, actual);
    }
}
