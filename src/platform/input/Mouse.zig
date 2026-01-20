const core = @import("core");
const input = @import("root.zig");
const std = @import("std");

const Action = input.Action;
const Vec2f = core.math.Vec2f;

const Self = @This();

pub const Button = enum {
    left,
    middle,
    right,
    forward,
    backward,
};
const button_count = @typeInfo(Button).@"enum".fields.len;

current: Vec2f = .zero,
last: Vec2f = .zero,
buttons: [button_count]Action = .{.release} ** button_count,
_last_buttons: [button_count]Action = .{.release} ** button_count,

pub fn update(self: *Self, x: f32, y: f32) void {
    self.last.copy(self.current);
    self.current = .init(x, y);
    @memcpy(&self._last_buttons, &self.buttons);
}

pub fn delta(self: Self) Vec2f {
    return self.current.sub(self.last);
}

pub fn pressed(self: Self, button: Button) bool {
    return self.buttons[@intFromEnum(button)] == .press;
}

pub fn justPressed(self: Self, button: Button) bool {
    const index = @intFromEnum(button);
    return self.buttons[index] == .press and self._last_buttons[index] == .release;
}

pub fn released(self: Self, button: Button) bool {
    return self.buttons[@intFromEnum(button)] == .release;
}

pub fn justReleased(self: Self, button: Button) bool {
    const index = @intFromEnum(button);
    return self.buttons[index] == .release and self._last_buttons[index] == .press;
}

pub fn didChange(self: Self, button: Button) bool {
    const index = @intFromEnum(button);
    return self.buttons[index] != self._last_buttons[index];
}
