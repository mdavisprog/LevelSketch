const std = @import("std");

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

pub fn update(self: *Self, x: i64, y: i64) void {
    self.last.copy(self.current);
    self.current.set(x, y);
}

pub fn delta(self: Self) Point {
    return self.current.sub(self.last);
}

pub fn pressed(self: Self, button: Button) bool {
    return self.buttons[@intFromEnum(button)] == .press;
}

pub fn released(self: Self, button: Button) bool {
    return self.buttons[@intFromEnum(button)] == .release;
}
