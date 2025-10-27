const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const module = b.addModule("root", .{
        .root_source_file = b.path("src/root.zig"),
        .target = target,
        .optimize = optimize,
    });

    module.addIncludePath(b.path("includes/"));
    module.addCSourceFile(.{
        .file = b.path("src/stb_image.c"),
        .flags = &.{
            "-std=c99",
        },
    });
}
