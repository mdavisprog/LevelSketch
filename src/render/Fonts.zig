const io = @import("io");
const render = @import("root.zig");
const std = @import("std");

const Font = render.Font;
const Renderer = render.Renderer;

const Self = @This();

pub const FontMap = std.StringHashMap(*Font);

fonts: FontMap,

/// Caller is responsible for returned memory.
pub fn toKey(allocator: std.mem.Allocator, path: []const u8, size: f32) ![]u8 {
    const name = std.fs.path.stem(path);
    const sizei: i32 = @intFromFloat(size);
    var buffer = [_]u8{0} ** 4;
    _ = try std.fmt.bufPrint(&buffer, "{}", .{sizei});
    return try std.fs.path.join(allocator, &.{ name, "_", &buffer });
}

pub fn init(gpa: std.mem.Allocator) !*Self {
    const fonts = FontMap.init(gpa);
    const result = try gpa.create(Self);
    result.* = .{
        .fonts = fonts,
    };
    return result;
}

pub fn deinit(self: *Self, gpa: std.mem.Allocator) void {
    var it = self.fonts.iterator();
    while (it.next()) |font| {
        gpa.free(font.key_ptr.*);
        font.value_ptr.*.deinit(gpa);
        gpa.destroy(font.value_ptr.*);
    }
    self.fonts.deinit();
}

/// Must be relative to the 'assets/fonts' directory. The name of the file without the
/// extensions will be used as the key for the font data.
pub fn loadFile(self: *Self, renderer: *Renderer, path: []const u8, size: f32) !*Font {
    const full_path = try io.fontsPath(renderer._gpa, path);

    const font = try Font.init(renderer, full_path, size);
    errdefer renderer._gpa.destroy(font);

    const key = try toKey(renderer._gpa, path, size);
    errdefer renderer._gpa.free(key);

    try self.fonts.put(key, font);

    return font;
}
