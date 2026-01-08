const clay = @import("clay");
const render = @import("render");

const Font = render.Font;

const Self = @This();

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
