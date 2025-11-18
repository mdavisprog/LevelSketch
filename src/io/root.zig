const std = @import("std");

pub fn exeRelativePath(allocator: std.mem.Allocator, paths: []const []const u8) ![]u8 {
    const exe_dir = try std.fs.selfExeDirPathAlloc(allocator);
    defer allocator.free(exe_dir);

    var list = try std.ArrayList([]const u8).initCapacity(allocator, 0);
    defer list.deinit(allocator);

    try list.append(allocator, exe_dir);
    try list.appendSlice(allocator, paths);

    return try std.fs.path.join(allocator, list.items);
}

pub fn fontsPath(allocator: std.mem.Allocator, path: []const u8) ![]u8 {
    return try exeRelativePath(allocator, &.{ "assets/fonts", path });
}

pub fn getContents(allocator: std.mem.Allocator, path: []const u8) ![]u8 {
    const file = try std.fs.openFileAbsolute(path, .{});
    defer file.close();

    const file_size = try file.getEndPos();
    var buffer: [1024]u8 = undefined;
    var reader = file.reader(&buffer);
    return try reader.interface.readAlloc(allocator, file_size);
}
