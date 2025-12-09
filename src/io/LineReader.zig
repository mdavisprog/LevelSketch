const std = @import("std");

/// Struct to manage reading lines from either a file or a buffer.
const Self = @This();

const buffer_size: usize = 1024;

_payload: Payload,

pub fn initFile(allocator: std.mem.Allocator, path: []const u8) !Self {
    var file: File = undefined;
    file.handle = try std.fs.openFileAbsolute(path, .{});
    file.buffer = try allocator.alloc(u8, buffer_size);
    file.reader = file.handle.reader(file.buffer);
    return .init(.{
        .file = file,
    });
}

pub fn initFixed(buffer: []const u8) Self {
    return .init(.{
        .fixed = .{
            .reader = .fixed(buffer),
        },
    });
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    switch (self._payload) {
        .file => |file| {
            file.handle.close();
            allocator.free(file.buffer);
        },
        else => {},
    }
}

pub fn readLine(self: *Self) !?[]const u8 {
    var reader: *std.Io.Reader = switch (self._payload) {
        .file => |*file| &file.reader.interface,
        .fixed => |*fixed| &fixed.reader,
    };

    var line = reader.peekDelimiterExclusive('\n') catch |err| switch (err) {
        error.EndOfStream => {
            return null;
        },
        else => {
            return err;
        },
    };

    reader.toss(std.mem.min(usize, &[2]usize{ line.len + 1, reader.bufferedLen() }));

    // Check if there is a trailing carriage return and adjust line.
    if (line.len > 0 and line[line.len - 1] == '\r') {
        line.len -= 1;
    }

    return line;
}

fn init(payload: Payload) Self {
    return .{
        ._payload = payload,
    };
}

const PayloadType = enum {
    file,
    fixed,
};

const Payload = union(PayloadType) {
    file: File,
    fixed: Fixed,
};

const File = struct {
    handle: std.fs.File,
    reader: std.fs.File.Reader,
    buffer: []u8,
};

const Fixed = struct {
    reader: std.Io.Reader,
};
