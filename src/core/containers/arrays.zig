const std = @import("std");

pub const StaticArrayListError = error{
    OutOfMemory,
};

pub fn StaticArrayList(comptime T: type, comptime size: usize) type {
    return struct {
        const Self = @This();

        pub const empty: Self = .{};

        items: []T = &.{},
        _data: [size]T = undefined,

        pub fn append(self: *Self, item: T) StaticArrayListError!void {
            if (self.items.len >= size) {
                return StaticArrayListError.OutOfMemory;
            }

            var index = self.items.len;
            self._data[index] = item;
            index += 1;
            self.items = self._data[0..index];
        }

        pub fn clear(self: *Self) void {
            self.items.len = 0;
        }
    };
}
