const core = @import("core");
const std = @import("std");

const Vec2f = core.math.Vec2f;

const Self = @This();

pub const Point = struct {
    pub const zero: Point = .{ .x = 0, .y = 0 };

    x: i64,
    y: i64,

    pub fn set(self: *Point, x: i64, y: i64) void {
        self.x = x;
        self.y = y;
    }

    pub fn copy(self: *Point, from: Point) void {
        self.x = from.x;
        self.y = from.y;
    }

    pub fn sub(self: Point, from: Point) Point {
        return Point{
            .x = self.x - from.x,
            .y = self.y - from.y,
        };
    }

    pub fn isZero(self: Point) bool {
        return self.x == 0 and self.y == 0;
    }

    pub fn toVec2f(self: Point) Vec2f {
        return .init(
            @floatFromInt(self.x),
            @floatFromInt(self.y),
        );
    }
};

pub const Button = enum(usize) {
    left,
    middle,
    right,
    unhandled,
};
const button_count = @typeInfo(Button).@"enum".fields.len;

pub const Action = enum {
    press,
    release,
    repeat,
};

current: Point = .zero,
last: Point = .zero,
buttons: [button_count]Action = .{.release} ** button_count,
_last_buttons: [button_count]Action = .{.release} ** button_count,

pub fn update(self: *Self, x: i64, y: i64) void {
    self.last.copy(self.current);
    self.current.set(x, y);
    @memcpy(&self._last_buttons, &self.buttons);
}

pub fn delta(self: Self) Point {
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
