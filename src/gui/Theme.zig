const clay = @import("clay");

const Self = @This();

pub const Colors = struct {
    background: clay.Color = .initu8(32, 32, 32, 255),
    control: clay.Color = .initu8(54, 54, 54, 255),
    hovered: clay.Color = .initu8(77, 77, 77, 255),
    pressed: clay.Color = .initu8(85, 85, 85, 255),
};

colors: Colors = .{},
