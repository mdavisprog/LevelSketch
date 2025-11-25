const std = @import("std");

var args: ?std.ArrayList([]const u8) = null;

pub fn init(allocator: std.mem.Allocator) !void {
    if (args != null) {
        return;
    }

    const arguments = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, arguments);

    args = try std.ArrayList([]const u8).initCapacity(allocator, arguments.len);
    for (arguments) |arg| {
        const copy = try allocator.dupe(u8, arg);
        try args.?.append(allocator, copy);
    }
}

pub fn deinit(allocator: std.mem.Allocator) void {
    if (args) |*_args| {
        for (_args.items) |arg| {
            allocator.free(arg);
        }

        _args.deinit(allocator);
        args = null;
    }
}

pub fn hasArg(arg: []const u8) bool {
    if (args == null) {
        return false;
    }

    const _args = args.?;
    for (_args.items) |item| {
        if (std.mem.eql(u8, item, arg)) {
            return true;
        }
    }

    return false;
}

/// Caller owns the returned memory
pub fn getArgValues(allocator: std.mem.Allocator, arg: []const u8) !?[][]const u8 {
    const _args = args orelse return null;

    var result = try std.ArrayList([]const u8).initCapacity(allocator, 0);
    var found = false;
    for (_args.items) |item| {
        if (!found) {
            if (std.mem.eql(u8, item, arg)) {
                found = true;
            }
        } else {
            if (std.mem.startsWith(u8, item, "--")) {
                break;
            }

            try result.append(allocator, item);
        }
    }

    return try result.toOwnedSlice(allocator);
}
