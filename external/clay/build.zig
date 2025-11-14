const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const native_module = b.createModule(.{
        .target = target,
        .optimize = optimize,
    });
    native_module.addIncludePath(b.path("includes"));
    native_module.addCSourceFile(.{
        .file = b.path("src/clay.c"),
        .flags = &.{
            "-std=c99",
            "-fno-sanitize=undefined",
        },
    });

    const public_module = b.addModule("root", .{
        .root_source_file = b.path("src/root.zig"),
        .target = target,
        .optimize = optimize,
        .imports = &.{
            .{ .name = "clayc", .module = native_module },
        },
    });
    public_module.addIncludePath(b.path("includes"));
}
