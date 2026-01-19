const io = @import("io");
const render = @import("root.zig");
const std = @import("std");

const Font = render.Font;
const Renderer = render.Renderer;

const Self = @This();

pub const FontMap = std.StringHashMap(*Font);
pub const FontHandleMap = std.AutoHashMap(Font.Handle, *Font);

fonts: FontMap,
default: Font.Handle = Font.Handle.invalid,
_font_handles: FontHandleMap,

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
    const font_handles = FontHandleMap.init(gpa);
    const result = try gpa.create(Self);
    result.* = .{
        .fonts = fonts,
        ._font_handles = font_handles,
    };
    return result;
}

pub fn deinit(self: *Self, gpa: std.mem.Allocator) void {
    self._font_handles.deinit();

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
pub fn loadFile(
    self: *Self,
    renderer: *Renderer,
    path: []const u8,
    size: f32,
    font_type: Font.Type,
) !*Font {
    const full_path = try io.fontsPath(renderer.allocator, path);
    defer renderer.allocator.free(full_path);

    var font = try Font.init(renderer.allocator, full_path);
    errdefer renderer.allocator.destroy(font);

    try font.load(renderer, size, font_type);

    const key = try toKey(renderer.allocator, path, size);
    errdefer renderer.allocator.free(key);

    font.handle = .generate();

    try self.fonts.put(key, font);
    try self._font_handles.put(font.handle, font);

    if (!self.default.isValid()) {
        self.default = font.handle;
    }

    return font;
}

pub fn getByHandle(self: Self, handle: Font.Handle) ?*Font {
    return self._font_handles.get(handle);
}
