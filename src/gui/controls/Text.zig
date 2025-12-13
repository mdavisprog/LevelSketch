const render = @import("render");

const Font = render.Font;

const Self = @This();

contents: []const u8,
font: *const Font,

pub fn init(contents: []const u8, font: *const Font) Self {
    return .{
        .contents = contents,
        .font = font,
    };
}
