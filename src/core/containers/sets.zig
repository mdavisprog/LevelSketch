const std = @import("std");

pub fn HashSetUnmanaged(comptime T: type) type {
    return struct {
        const Self = @This();
        const Keys = std.AutoHashMapUnmanaged(T, void);

        pub const empty: Self = .{};

        _data: Keys = .empty,

        pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
            self._data.deinit(allocator);
        }

        pub fn contains(self: Self, key: T) bool {
            return self._data.contains(key);
        }

        pub fn count(self: Self) u32 {
            return self._data.count();
        }

        pub fn isEmpty(self: Self) bool {
            return self.count() == 0;
        }

        pub fn insert(self: *Self, allocator: std.mem.Allocator, key: T) !void {
            try self._data.put(allocator, key, {});
        }

        pub fn remove(self: *Self, key: T) bool {
            return self._data.remove(key);
        }

        pub fn iterator(self: Self) Keys.KeyIterator {
            return self._data.keyIterator();
        }

        pub fn clearAndFree(self: *Self, allocator: std.mem.Allocator) void {
            self._data.clearAndFree(allocator);
        }

        pub fn clearRetainingCapacity(self: *Self) void {
            self._data.clearRetainingCapacity();
        }

        pub fn setFrom(self: *Self, allocator: std.mem.Allocator, other: Self) !void {
            var it = other.iterator();
            while (it.next()) |element| {
                try self.insert(allocator, element.*);
            }
        }
    };
}

test "insert" {
    const allocator = std.testing.allocator;

    var set = HashSetUnmanaged(u32).empty;
    defer set.deinit(allocator);

    try set.insert(allocator, 5);
    try set.insert(allocator, 18);
    try set.insert(allocator, 42);

    try std.testing.expectEqual(3, set.count());
}

test "remove" {
    const allocator = std.testing.allocator;

    var set = HashSetUnmanaged(u32).empty;
    defer set.deinit(allocator);

    try set.insert(allocator, 5);
    try set.insert(allocator, 18);
    try set.insert(allocator, 42);

    try std.testing.expectEqual(3, set.count());

    try std.testing.expectEqual(true, set.remove(18));
    try std.testing.expectEqual(2, set.count());

    try std.testing.expectEqual(false, set.remove(1));
    try std.testing.expectEqual(2, set.count());
}
