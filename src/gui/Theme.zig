const clay = @import("clay");
const io = @import("io");
const render = @import("render");
const std = @import("std");

const Font = render.Font;
const Renderer = render.Renderer;
const Texture = render.Texture;

const Self = @This();

pub const IconMap = std.StringHashMapUnmanaged(Texture);

pub const Colors = struct {
    background: clay.Color = .initu8(32, 32, 32, 255),
    control: clay.Color = .initu8(54, 54, 54, 255),
    hovered: clay.Color = .initu8(77, 77, 77, 255),
    pressed: clay.Color = .initu8(85, 85, 85, 255),
};

pub const FontSizes = struct {
    header: u16 = 22,
    normal: u16 = 18,
};

colors: Colors = .{},
font: *const Font,
font_sizes: FontSizes = .{},
icons: IconMap = .empty,
_default_texture: Texture,

pub fn init(renderer: *Renderer, font: *const Font) !Self {
    const allocator = renderer.allocator;

    const icons_path = try io.exeRelativePath(allocator, &.{"assets/icons"});
    defer allocator.free(icons_path);

    var dir = try std.fs.openDirAbsolute(icons_path, .{ .iterate = true });
    defer dir.close();

    const width: usize = 16;
    const height: usize = 16;
    var icons: IconMap = .empty;

    var it = dir.iterate();
    while (try it.next()) |entry| {
        const path = try std.fs.path.join(allocator, &.{ icons_path, entry.name });
        defer allocator.free(path);

        const contents = try io.loadSVG(allocator, path, width, height);
        const texture = try renderer.textures.loadBuffer(
            &renderer.mem_factory,
            contents,
            width,
            height,
            .rgba8,
        );

        const key = try allocator.dupe(u8, std.fs.path.stem(entry.name));
        try icons.put(allocator, key, texture);
    }

    return .{
        .font = font,
        .icons = icons,
        ._default_texture = renderer.textures.default,
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    var keys = self.icons.keyIterator();
    while (keys.next()) |key| {
        allocator.free(key.*);
    }
    self.icons.deinit(allocator);
}

pub fn getIcon(self: Self, name: []const u8) Texture {
    return self.icons.get(name) orelse self._default_texture;
}
