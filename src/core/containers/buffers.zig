const math = @import("../math/root.zig");
const std = @import("std");

const Rectus = math.Rectus;
const Vec2us = math.Vec2us;

pub const Error = error{
    OutOfBounds,
    IncorrectExpandSize,
};

pub fn Buffer2D(comptime T: type) type {
    return struct {
        const Self = @This();

        data: std.ArrayList(T),
        width: usize,
        height: usize,
        _allocator: std.mem.Allocator,

        pub fn init(allocator: std.mem.Allocator, width: usize, height: usize, default: T) !Self {
            var data = try std.ArrayList(T).initCapacity(allocator, 0);
            try data.appendNTimes(allocator, default, width * height);
            return Self{
                .data = data,
                .width = width,
                .height = height,
                ._allocator = allocator,
            };
        }

        pub fn deinit(self: *Self) void {
            self.data.deinit(self._allocator);
        }

        pub fn resize(self: *Self, width: usize, height: usize, default: T) !void {
            try self.expand(width, height);
            @memset(self.data.items, default);
        }

        pub fn expand(self: *Self, width: usize, height: usize) !void {
            if (width <= self.width or height <= self.height) {
                return Error.IncorrectExpandSize;
            }

            try self.data.resize(self._allocator, width * height);
            self.width = width;
            self.height = height;
        }

        pub fn put(self: *Self, x: usize, y: usize, value: T) Error!void {
            const index = try self.getIndex(x, y);
            self.data.items[index] = value;
        }

        pub fn putRegion(self: *Self, region: Rectus, value: []const T) Error!void {
            var src: usize = 0;
            var y = region.min.y;
            while (y <= region.max.y) {
                var x = region.min.x;
                while (x <= region.max.x) {
                    const index = try self.getIndex(x, y);
                    self.data.items[index] = value[src];

                    x += 1;
                    src += 1;
                }

                y += 1;
            }
        }

        pub fn get(self: Self, x: usize, y: usize) !T {
            const index = try self.getIndex(x, y);
            return self.data.items[index];
        }

        /// Caller is responsible for returned memory.
        pub fn getRegion(self: Self, region: Rectus) ![]T {
            const stride: Vec2us = .init(region.width() + 1, region.height() + 1);
            var result = try self._allocator.alloc(T, stride.x * stride.y);

            var dst: usize = 0;
            var y = region.min.y;
            while (y <= region.max.y) {
                var x = region.min.x;
                while (x <= region.max.x) {
                    const index = try self.getIndex(x, y);
                    result[dst] = self.data.items[index];

                    x += 1;
                    dst += 1;
                }
                y += 1;
            }

            return result;
        }

        pub fn copyRegion(
            self: *Self,
            dst_region: Rectus,
            src: Self,
            src_region: Rectus,
        ) !void {
            var src_y = src_region.min.y;
            var dst_y = dst_region.min.y;
            while (src_y <= src_region.max.y) {
                var src_x = src_region.min.x;
                var dst_x = dst_region.min.x;
                while (src_x <= src_region.max.x) {
                    const src_pixel = try src.get(src_x, src_y);
                    try self.put(dst_x, dst_y, src_pixel);

                    src_x += 1;
                    dst_x += 1;
                }

                src_y += 1;
                dst_y += 1;
            }
        }

        pub fn getIndex(self: Self, x: usize, y: usize) Error!usize {
            if (x >= self.width or y >= self.height) {
                return Error.OutOfBounds;
            }

            return y * self.width + x;
        }

        pub fn getSize(self: Self) Vec2us {
            return .init(self.width, self.height);
        }

        pub fn release(self: *Self) !Self {
            const data = try self.data.toOwnedSlice(self._allocator);
            const width = self.width;
            const height = self.height;
            self.width = 0;
            self.height = 0;
            return Self{
                .data = std.ArrayList(u8).fromOwnedSlice(data),
                .width = width,
                .height = height,
                ._allocator = self._allocator,
            };
        }
    };
}

test "initialize" {
    const allocator = std.testing.allocator;
    var buffer = try Buffer2D(u8).init(allocator, 2, 2, 4);
    defer buffer.deinit();
    for (buffer.data.items) |item| {
        try std.testing.expectEqual(4, item);
    }
}

test "put" {
    const allocator = std.testing.allocator;
    var buffer = try Buffer2D(u8).init(allocator, 2, 2, 0);
    defer buffer.deinit();

    try buffer.put(0, 1, 1);
    try std.testing.expectEqual(0, buffer.data.items[0]);
    try std.testing.expectEqual(0, buffer.data.items[1]);
    try std.testing.expectEqual(1, buffer.data.items[2]);
    try std.testing.expectEqual(0, buffer.data.items[3]);

    try std.testing.expectEqual(Error.OutOfBounds, buffer.put(4, 4, 0));
}

test "putRegion" {
    const allocator = std.testing.allocator;
    var buffer = try Buffer2D(u8).init(allocator, 3, 3, 0);
    defer buffer.deinit();

    const pixel: Rectus = .init(0, 0, 0, 0);
    try buffer.putRegion(pixel, &.{1});

    const region: Rectus = .init(1, 1, 2, 2);
    try buffer.putRegion(region, &.{ 1, 2, 3, 4 });
    try std.testing.expectEqual(1, buffer.data.items[0]);
    try std.testing.expectEqual(0, buffer.data.items[1]);
    try std.testing.expectEqual(0, buffer.data.items[2]);
    try std.testing.expectEqual(0, buffer.data.items[3]);
    try std.testing.expectEqual(1, buffer.data.items[4]);
    try std.testing.expectEqual(2, buffer.data.items[5]);
    try std.testing.expectEqual(0, buffer.data.items[6]);
    try std.testing.expectEqual(3, buffer.data.items[7]);
    try std.testing.expectEqual(4, buffer.data.items[8]);
    try std.testing.expectEqual(2, try buffer.get(2, 1));
}

test "resize" {
    const allocator = std.testing.allocator;
    var buffer = try Buffer2D(u8).init(allocator, 2, 2, 1);
    defer buffer.deinit();

    try buffer.resize(3, 3, 4);
    try std.testing.expectEqual(9, buffer.data.items.len);
    for (buffer.data.items) |item| {
        try std.testing.expectEqual(4, item);
    }

    try std.testing.expectEqual(Error.IncorrectExpandSize, buffer.expand(1, 1));
}

test "getRegion" {
    const allocator = std.testing.allocator;
    var buffer = try Buffer2D(u8).init(allocator, 4, 4, 0);
    defer buffer.deinit();

    const data = [_]u8{ 1, 2, 3, 4 };
    try buffer.putRegion(.init(1, 1, 2, 2), &data);

    const region = try buffer.getRegion(.init(0, 1, 1, 2));
    defer allocator.free(region);

    try std.testing.expectEqual(4, region.len);
    try std.testing.expectEqual(0, region[0]);
    try std.testing.expectEqual(1, region[1]);
    try std.testing.expectEqual(0, region[2]);
    try std.testing.expectEqual(3, region[3]);
}

test "copyRegion" {
    const allocator = std.testing.allocator;

    var buffer_1 = try Buffer2D(u8).init(allocator, 3, 3, 0);
    defer buffer_1.deinit();

    try buffer_1.put(0, 0, 1);
    try buffer_1.put(1, 0, 2);
    try buffer_1.put(2, 0, 3);
    try buffer_1.put(0, 1, 4);
    try buffer_1.put(1, 1, 5);
    try buffer_1.put(2, 1, 6);
    try buffer_1.put(0, 2, 7);
    try buffer_1.put(1, 2, 8);
    try buffer_1.put(2, 2, 9);

    var buffer_2 = try Buffer2D(u8).init(allocator, 3, 3, 0);
    defer buffer_2.deinit();

    try buffer_2.put(0, 0, 9);
    try buffer_2.put(1, 0, 8);
    try buffer_2.put(2, 0, 7);
    try buffer_2.put(0, 1, 6);
    try buffer_2.put(1, 1, 5);
    try buffer_2.put(2, 1, 4);
    try buffer_2.put(0, 2, 3);
    try buffer_2.put(1, 2, 2);
    try buffer_2.put(2, 2, 1);

    const src: Rectus = .init(1, 1, 2, 2);
    const dst: Rectus = .init(0, 0, 1, 1);

    try buffer_1.copyRegion(dst, buffer_2, src);

    try std.testing.expectEqual(5, buffer_1.get(0, 0));
    try std.testing.expectEqual(4, buffer_1.get(1, 0));
    try std.testing.expectEqual(3, buffer_1.get(2, 0));
    try std.testing.expectEqual(2, buffer_1.get(0, 1));
    try std.testing.expectEqual(1, buffer_1.get(1, 1));
    try std.testing.expectEqual(6, buffer_1.get(2, 1));
    try std.testing.expectEqual(7, buffer_1.get(0, 2));
    try std.testing.expectEqual(8, buffer_1.get(1, 2));
    try std.testing.expectEqual(9, buffer_1.get(2, 2));
}

test "release" {
    const allocator = std.testing.allocator;
    var buffer = try Buffer2D(u8).init(allocator, 2, 2, 2);
    try std.testing.expectEqual(4, buffer.data.items.len);

    var owned = try buffer.release();
    defer owned.deinit();

    try std.testing.expectEqual(4, owned.data.items.len);
    try std.testing.expectEqual(0, buffer.data.items.len);
}
