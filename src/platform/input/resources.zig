pub const input = @import("root.zig");

pub const Mouse = struct {
    state: input.Mouse = .{},
};

pub const Keyboard = struct {
    state: input.Keyboard = .{},
};
