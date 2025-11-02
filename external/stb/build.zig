const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const module = b.addModule("root", .{
        .root_source_file = b.path("src/root.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    module.addIncludePath(b.path("includes/"));
    module.addCSourceFiles(.{
        .files = &.{
            "src/stb.c",
        },
        .flags = &.{
            "-std=c99",
            "-fno-sanitize=undefined",
        },
    });
}
