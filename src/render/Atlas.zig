const core = @import("core");
const std = @import("std");

const Buffer2D = core.containers.Buffer2D(u8);
const Rectus = core.math.Rectus;
const Vec2f = core.math.Vec2f;
const Vec2us = core.math.Vec2us;

/// A growable 2D buffer that keeps track of regions of data that have been placed.
/// Currently, the packing is extremely inefficient as each region is just added in order. If the
/// given region can't fit, the atlas is resized and all previous regions are packed again
/// before continuing on.
const Self = @This();

pub const Error = error{
    TooManyIterations,
};

pub const Cursor = struct {
    pos: Vec2us = .ZERO,
    size: Vec2us = .ZERO,
    height: usize = 0,

    fn canFit(self: Cursor, size: Vec2us) bool {
        const max: Vec2us = .init(
            self.pos.x + size.x - 1,
            self.pos.y + size.y - 1,
        );

        if (max.x >= self.size.x) {
            return false;
        }

        if (max.y >= self.size.y) {
            return false;
        }

        return true;
    }

    fn ensureFit(self: *Cursor, size: Vec2us) bool {
        if (!self.canFit(size)) {
            self.advance(size);
        }

        return self.canFit(size);
    }

    fn advance(self: *Cursor, size: Vec2us) void {
        if (self.canFit(size)) {
            self.pos.x += size.x;
        } else {
            self.pos.x = 0;
            self.pos.y += self.height;
            self.height = 0;
        }
    }

    fn updateHeight(self: *Cursor, height: usize) void {
        self.*.height = @max(self.height, height);
    }
};

buffer: Buffer2D,
regions: std.ArrayList(Rectus),
base_size: Vec2us = Vec2us.ZERO,
resize_scale: Vec2f = .init(2.0, 2.0),
_cursor: Cursor = .{},
_allocator: std.mem.Allocator,

pub fn init(allocator: std.mem.Allocator, width: usize, height: usize) !Self {
    const buffer = try Buffer2D.init(
        allocator,
        width,
        height,
        0,
    );
    const regions = try std.ArrayList(Rectus).initCapacity(allocator, 0);
    return Self{
        .buffer = buffer,
        .base_size = .init(width, height),
        .regions = regions,
        ._allocator = allocator,
        ._cursor = .{ .size = .init(width, height) },
    };
}

pub fn deinit(self: *Self) void {
    self.buffer.deinit();
    self.regions.deinit(self._allocator);
}

/// For placing a single pixel, use a size of (0, 0).
pub fn place(self: *Self, size: Vec2us, data: []const u8) !void {
    // See if the given region can fit within the current bounds.
    if (!self._cursor.ensureFit(size)) {
        var new_size = self.buffer.getSize();

        var iterations: u32 = 0;
        while (true) {
            new_size = self.getExpandSize(new_size);
            if (try self.expandAndRepack(new_size)) {
                if (self._cursor.ensureFit(size)) {
                    break;
                }
            }

            iterations += 1;
            if (iterations >= 10) {
                return Error.TooManyIterations;
            }
        }
    }

    const region: Rectus = .init(
        self._cursor.pos.x,
        self._cursor.pos.y,
        self._cursor.pos.x + size.x -| 1,
        self._cursor.pos.y + size.y -| 1,
    );

    try self.*.buffer.putRegion(region, data);
    try self.regions.append(self._allocator, region);

    self._cursor.updateHeight(size.y);
    self._cursor.advance(size);
}

fn expandAndRepack(self: *Self, new_size: Vec2us) !bool {
    var dst = try Buffer2D.init(self._allocator, new_size.x, new_size.y, 0);
    defer dst.deinit();

    var dst_regions = try std.ArrayList(Rectus).initCapacity(self._allocator, 0);
    defer dst_regions.deinit(self._allocator);

    var dst_cursor: Cursor = .{ .size = new_size };
    for (self.regions.items) |src_region| {
        const region_size: Vec2us = .init(src_region.width() + 1, src_region.height() + 1);

        if (!dst_cursor.ensureFit(region_size)) {
            return false;
        }

        const dst_region: Rectus = .init(
            dst_cursor.pos.x,
            dst_cursor.pos.y,
            dst_cursor.pos.x + src_region.width(),
            dst_cursor.pos.y + src_region.height(),
        );

        try dst.copyRegion(dst_region, self.buffer, src_region);

        dst_cursor.updateHeight(region_size.y);
        dst_cursor.advance(region_size);
        try dst_regions.append(self._allocator, dst_region);
    }

    self.buffer.deinit();
    self.buffer = try dst.release();

    const dst_regions_data = try dst_regions.toOwnedSlice(self._allocator);
    self.regions.deinit(self._allocator);
    self.regions = std.ArrayList(Rectus).fromOwnedSlice(dst_regions_data);

    self._cursor = dst_cursor;

    return true;
}

fn getExpandSize(self: Self, size: Vec2us) Vec2us {
    const width: f32 = @floatFromInt(size.x);
    const height: f32 = @floatFromInt(size.y);

    const new_width = width * self.resize_scale.x;
    const new_height = height * self.resize_scale.y;

    return .init(@intFromFloat(new_width), @intFromFloat(new_height));
}

test "place" {
    const allocator = std.testing.allocator;
    var atlas: Self = try .init(allocator, 2, 2);
    defer atlas.deinit();

    try atlas.place(.init(1, 1), &.{1});
    try atlas.place(.init(2, 1), &.{ 2, 3 });
    try std.testing.expectEqual(1, try atlas.buffer.get(0, 0));
    try std.testing.expectEqual(0, try atlas.buffer.get(1, 0));
    try std.testing.expectEqual(2, try atlas.buffer.get(0, 1));
    try std.testing.expectEqual(3, try atlas.buffer.get(1, 1));
}

test "placeExpand" {
    const allocator = std.testing.allocator;
    var atlas: Self = try .init(allocator, 2, 2);
    defer atlas.deinit();

    try atlas.place(.init(2, 2), &.{ 1, 2, 3, 4 });
    try std.testing.expectEqual(4, atlas.buffer.data.items.len);

    try atlas.place(.init(2, 2), &.{ 5, 6, 7, 8 });
    try std.testing.expectEqual(16, atlas.buffer.data.items.len);

    try std.testing.expectEqual(1, try atlas.buffer.get(0, 0));
    try std.testing.expectEqual(2, try atlas.buffer.get(1, 0));
    try std.testing.expectEqual(5, try atlas.buffer.get(2, 0));
    try std.testing.expectEqual(6, try atlas.buffer.get(3, 0));
    try std.testing.expectEqual(3, try atlas.buffer.get(0, 1));
    try std.testing.expectEqual(4, try atlas.buffer.get(1, 1));
    try std.testing.expectEqual(7, try atlas.buffer.get(2, 1));
    try std.testing.expectEqual(8, try atlas.buffer.get(3, 1));
    try std.testing.expectEqual(0, try atlas.buffer.get(0, 2));
    try std.testing.expectEqual(0, try atlas.buffer.get(1, 2));
    try std.testing.expectEqual(0, try atlas.buffer.get(2, 2));
    try std.testing.expectEqual(0, try atlas.buffer.get(3, 2));
    try std.testing.expectEqual(0, try atlas.buffer.get(0, 3));
    try std.testing.expectEqual(0, try atlas.buffer.get(1, 3));
    try std.testing.expectEqual(0, try atlas.buffer.get(2, 3));
    try std.testing.expectEqual(0, try atlas.buffer.get(3, 3));

    std.debug.print("=== atlas2 test ===\n", .{});
    var atlas2: Self = try .init(allocator, 2, 2);
    defer atlas2.deinit();

    try atlas2.place(.init(2, 2), &.{ 1, 2, 3, 4 });
    try std.testing.expectEqual(4, atlas2.buffer.data.items.len);

    try atlas2.place(.init(3, 2), &.{ 5, 6, 7, 8, 9, 0 });
    try std.testing.expectEqual(16, atlas2.buffer.data.items.len);
}

test "expand" {
    const allocator = std.testing.allocator;
    var atlas: Self = try .init(allocator, 64, 64);
    defer atlas.deinit();

    const expand_size = atlas.getExpandSize(atlas.buffer.getSize());
    try std.testing.expectEqual(128, expand_size.x);
    try std.testing.expectEqual(128, expand_size.y);
}

test "tooManyIterations" {
    const allocator = std.testing.allocator;
    var atlas: Self = try .init(allocator, 1, 1);
    defer atlas.deinit();

    const width: usize = 10000;
    const height: usize = 10000;
    const data = try allocator.alloc(u8, width * height);
    defer allocator.free(data);

    @memset(data, 0);

    try std.testing.expectEqual(
        Error.TooManyIterations,
        atlas.place(.init(width, height), data),
    );
}
