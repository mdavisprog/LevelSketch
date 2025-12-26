const std = @import("std");

pub fn writePGM(path: []const u8, data: []const u8, width: usize, height: usize) !void {
    return write(path, data, width, height, .P5);
}

pub fn writePPM(path: []const u8, data: []const u8, width: usize, height: usize) !void {
    return write(path, data, width, height, .P6);
}

const Format = enum {
    P1,
    P2,
    P3,
    P4,
    P5,
    P6,
};

fn write(
    path: []const u8,
    data: []const u8,
    width: usize,
    height: usize,
    format: Format,
) !void {
    var file = try std.fs.createFileAbsolute(path, .{});
    defer file.close();

    var buffer: [1024]u8 = undefined;
    var writer = file.writer(&buffer);

    try writer.interface.print("{s}\n", .{@tagName(format)});
    try writer.interface.print("{} {}\n", .{ width, height });

    switch (format) {
        .P2, .P3, .P5, .P6 => {
            try writer.interface.print("255\n", .{});
        },
        else => {},
    }

    try writer.interface.writeAll(data);
}
