const std = @import("std");

pub const Error = error{
    FailedToLoad,
};

/// Caller is responsible for returned data.
pub fn loadFile(
    allocator: std.mem.Allocator,
    path: []const u8,
    width: usize,
    height: usize,
) ![]u8 {
    const result = load(path.ptr, @intCast(width), @intCast(height));
    if (result == null) {
        return Error.FailedToLoad;
    }

    const length: usize = @intCast(width * height * 4);
    const data = try allocator.alloc(u8, length);
    @memcpy(data, result[0..length]);
    std.c.free(@ptrCast(result));
    return data;
}

extern fn load(path: [*c]const u8, width: c_int, height: c_int) [*c]u8;
