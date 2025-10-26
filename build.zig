const std = @import("std");
const zon = @import("build.zig.zon");

pub fn build(b: *std.Build) void {
    const version_string = zon.version;
    const version = std.SemanticVersion.parse(version_string) catch |err| {
        std.log.err("Failed to parse semantic version from zon.\n{any}", .{err});
        return;
    };

    const version_options = b.addOptions();
    version_options.addOption([]const u8, "full", version_string);
    version_options.addOption(usize, "major", version.major);
    version_options.addOption(usize, "minor", version.minor);
    version_options.addOption(usize, "patch", version.patch);

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const zglfw = b.dependency("zglfw", .{
        .target = target,
        .optimize = optimize,
    });

    const zbgfx = b.dependency("zbgfx", .{
        .target = target,
        .optimize = optimize,
    });

    const zmath = b.dependency("zmath", .{
        .target = target,
        .optimize = optimize,
    });

    const core = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .root_source_file = b.path("src/core/root.zig"),
    });

    const render = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .root_source_file = b.path("src/render/root.zig"),
        .imports = &.{
            .{ .name = "zbgfx", .module = zbgfx.module("zbgfx") },
            .{ .name = "zmath", .module = zmath.module("root") },
        },
    });

    const exe = b.addExecutable(.{
        .name = "LevelSketch",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
            .imports = &.{
                .{ .name = "core", .module = core },
                .{ .name = "render", .module = render },
                .{ .name = "zglfw", .module = zglfw.module("root") },
                .{ .name = "zbgfx", .module = zbgfx.module("zbgfx") },
                .{ .name = "zmath", .module = zmath.module("root") },
            },
        }),
    });
    exe.root_module.addOptions("version", version_options);

    if (target.result.os.tag != .emscripten) {
        exe.linkLibrary(zglfw.artifact("glfw"));
    }

    exe.linkLibrary(zbgfx.artifact("bgfx"));

    b.installArtifact(exe);
    b.installArtifact(zbgfx.artifact("shaderc"));

    const install_shaders = b.addInstallDirectory(.{
        .install_dir = .bin,
        .install_subdir = "assets/shaders/include",
        .source_dir = zbgfx.path("shaders"),
    });
    exe.step.dependOn(&install_shaders.step);

    const app_assets = b.addInstallDirectory(.{
        .install_dir = .bin,
        .install_subdir = "assets",
        .source_dir = b.path("assets"),
    });
    exe.step.dependOn(&app_assets.step);

    const run_step = b.step("run", "Run the app");
    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const render_tests = b.addTest(.{
        .root_module = render,
    });

    const exe_tests = b.addTest(.{
        .root_module = exe.root_module,
    });

    const run_render_tests = b.addRunArtifact(render_tests);
    const run_exe_tests = b.addRunArtifact(exe_tests);

    const test_step = b.step("test", "Run tests");
    test_step.dependOn(&run_render_tests.step);
    test_step.dependOn(&run_exe_tests.step);
}
