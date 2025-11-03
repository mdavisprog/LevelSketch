const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const use_llvm = b.option(bool, "use-llvm", "LLVM Zig backend");

    const tt_mod = b.addModule("TrueType", .{
        .root_source_file = b.path("TrueType.zig"),
        .target = target,
        .optimize = optimize,
    });

    const stb_truetype_object = b.addObject(.{
        .name = "stb_truetype",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        }),
    });
    stb_truetype_object.addCSourceFile(.{
        .file = b.path("test/stb_truetype.c"),
    });
    const stb_truetype_bindings = b.addTranslateC(.{
        .root_source_file = b.path("test/stb_truetype.h"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const unit_tests = b.addTest(.{
        .root_module = b.createModule(.{
            .root_source_file = b.path("test/test.zig"),
            .target = target,
            .optimize = optimize,
        }),
        .use_llvm = use_llvm,
        .use_lld = use_llvm,
    });
    unit_tests.filters = b.option([]const []const u8, "test-filter", "string to match for unit tests") orelse &.{};
    unit_tests.root_module.addImport("TrueType", tt_mod);
    unit_tests.root_module.addImport("c", stb_truetype_bindings.createModule());
    unit_tests.addObject(stb_truetype_object);

    const run_unit_tests = b.addRunArtifact(unit_tests);
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_unit_tests.step);
}
