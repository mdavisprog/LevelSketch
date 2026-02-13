const core = @import("core");
const io = @import("../root.zig");
const std = @import("std");

const Vec = core.math.Vec;

const LineReader = io.LineReader;

const Self = @This();

name: []const u8 = "",
ambient: Vec = .zero,
diffuse: Vec = .splat(1.0),
specular: Vec = .zero,
specular_exponent: f32 = 0.0,
diffuse_texture: ?[]const u8 = null,
specular_texture: ?[]const u8 = null,

/// 'path' must be an absolute path.
pub fn loadFile(allocator: std.mem.Allocator, path: []const u8) ![]Self {
    var reader: LineReader = try .initFile(allocator, path);
    defer reader.deinit(allocator);

    const result = try process(allocator, &reader);
    try resolve(allocator, path, result);

    return result;
}

pub fn loadData(allocator: std.mem.Allocator, data: []const u8) ![]Self {
    var reader: LineReader = .initFixed(data);

    const result = try process(allocator, &reader);
    try resolve(allocator, "", result);

    return result;
}

pub fn init(name: []const u8) Self {
    return .{
        .name = name,
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    allocator.free(self.name);

    if (self.diffuse_texture) |texture| allocator.free(texture);
    if (self.specular_texture) |texture| allocator.free(texture);
}

fn process(allocator: std.mem.Allocator, reader: *LineReader) ![]Self {
    var result: std.ArrayList(Self) = try .initCapacity(allocator, 0);

    while (try reader.readLine()) |line| {
        const material: ?*Self = if (result.items.len > 0)
            &result.items[result.items.len - 1]
        else
            null;

        var tokens = std.mem.tokenizeAny(u8, line, " ");
        const tag = tokens.next() orelse continue;

        if (std.mem.eql(u8, tag, "newmtl")) {
            if (tokens.next()) |name| {
                try result.append(allocator, .init(try allocator.dupe(u8, name)));
            }
        } else if (std.mem.eql(u8, tag, "Ka")) {
            if (material) |mat| {
                mat.ambient = processVec(line);
            }
        } else if (std.mem.eql(u8, tag, "Kd")) {
            if (material) |mat| {
                mat.diffuse = processVec(line);
            }
        } else if (std.mem.eql(u8, tag, "Ks")) {
            if (material) |mat| {
                mat.specular = processVec(line);
            }
        } else if (std.mem.eql(u8, tag, "Ns")) {
            if (tokens.next()) |ns| {
                if (material) |mat| {
                    mat.specular_exponent = std.fmt.parseFloat(f32, ns) catch 0.0;
                }
            }
        } else if (std.mem.eql(u8, tag, "map_Kd")) {
            if (tokens.next()) |file| {
                if (material) |mat| {
                    mat.diffuse_texture = try allocator.dupe(u8, file);
                }
            }
        } else if (std.mem.eql(u8, tag, "map_Ks")) {
            if (tokens.next()) |file| {
                if (material) |mat| {
                    mat.specular_texture = try allocator.dupe(u8, file);
                }
            }
        }
    }

    return try result.toOwnedSlice(allocator);
}

fn resolve(allocator: std.mem.Allocator, path: []const u8, materials: []Self) !void {
    // Resolve path names for textures.
    for (materials) |*mat| {
        mat.diffuse_texture = try resolvePath(allocator, path, mat.diffuse_texture);
        mat.specular_texture = try resolvePath(allocator, path, mat.specular_texture);
    }
}

/// Given path should be a relative path.
fn resolvePath(
    allocator: std.mem.Allocator,
    material_path: []const u8,
    path: ?[]const u8,
) !?[]const u8 {
    const path_ = path orelse return null;
    if (std.fs.path.isAbsolute(path_)) {
        return path_;
    }

    // Relative path needs to be transformed. Free the original string.
    defer allocator.free(path_);

    const directory = if (std.fs.path.dirname(material_path)) |mat_path|
        try allocator.dupe(u8, mat_path)
    else
        try std.fs.cwd().realpathAlloc(allocator, ".");
    defer allocator.free(directory);

    return try std.fs.path.join(allocator, &.{ directory, path_ });
}

fn processVec(line: []const u8) Vec {
    var it = std.mem.tokenizeAny(u8, line, " ");

    // Skip the key character.
    _ = it.next();

    const x = std.fmt.parseFloat(f32, it.next() orelse "") catch 0.0;
    const y = std.fmt.parseFloat(f32, it.next() orelse "") catch 0.0;
    const z = std.fmt.parseFloat(f32, it.next() orelse "") catch 0.0;
    return .init(x, y, z, 0.0);
}

test "parse mtl data" {
    const data =
        \\newmtl test
        \\map_Kd test.png
        \\Ka 0.2 0.45 0.89
        \\kd 1.0 1.0 1.0
        \\
        \\newmtl test2
        \\map_Kd test2.tga
        \\Kd 1.0 0.8 0.5
        \\Ks 0.5 0.4 0.3
        \\Ns 32.0
    ;

    const allocator = std.testing.allocator;
    const materials = try loadData(allocator, data);
    defer {
        for (materials) |*material| {
            material.deinit(allocator);
        }
        allocator.free(materials);
    }

    try std.testing.expectEqual(2, materials.len);
    try std.testing.expectEqualStrings("test", materials[0].name);
    try std.testing.expectEqualStrings("test2", materials[1].name);

    try std.testing.expectEqualStrings("test.png", std.fs.path.basename(materials[0].diffuse_texture.?));
    try std.testing.expectEqualStrings("test2.tga", std.fs.path.basename(materials[1].diffuse_texture.?));

    try std.testing.expect(materials[0].ambient.eql(.init(0.2, 0.45, 0.89, 0.0)));
    try std.testing.expect(materials[0].diffuse.eql(.splat(1.0)));

    try std.testing.expect(materials[1].diffuse.eql(.init(1.0, 0.8, 0.5, 0.0)));
    try std.testing.expect(materials[1].specular.eql(.init(0.5, 0.4, 0.3, 0.0)));
    try std.testing.expectEqual(32.0, materials[1].specular_exponent);
}
