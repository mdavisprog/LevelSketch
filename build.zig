const std = @import("std");
const zon = @import("build.zig.zon");

pub fn build(b: *std.Build) !void {
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

    var gpa = std.heap.GeneralPurposeAllocator(.{}).init;
    const allocator = gpa.allocator();

    var builder = Builder.init(allocator, b);
    defer builder.deinit();

    try builder.addDependency("clay", "root");
    try builder.addDependency("lunasvg", "root");
    try builder.addDependency("stb", "root");
    try builder.addDependency("zglfw", "root");
    try builder.addDependency("zbgfx", "zbgfx");
    try builder.addDependency("zmath", "root");

    try builder.addModule("core", "src/core/root.zig", &.{
        "zmath",
    });
    try builder.addModule("io", "src/io/root.zig", &.{
        "core",
        "lunasvg",
    });
    try builder.addModule("world", "src/world/root.zig", &.{
        "core",
    });
    try builder.addModule("render", "src/render/root.zig", &.{
        "core",
        "io",
        "stb",
        "zbgfx",
        "zmath",
        "world",
    });
    try builder.addModule("app", "src/app/root.zig", &.{});
    try builder.addModule("gui", "src/gui/root.zig", &.{
        "app",
        "clay",
        "core",
        "io",
        "render",
        "zbgfx",
        "world",
    });

    const root_module = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = builder.target,
        .optimize = builder.optimize,
    });
    root_module.addOptions("version", version_options);
    root_module.addIncludePath(b.path("external"));

    const exe = b.addExecutable(.{
        .name = "LevelSketch",
        .root_module = root_module,
    });
    builder.importAll(exe);

    if (builder.target.result.os.tag != .emscripten) {
        exe.linkLibrary(try builder.dependencyArtifact("zglfw", "glfw"));
    }

    exe.linkLibrary(try builder.dependencyArtifact("zbgfx", "bgfx"));
    exe.linkLibrary(try builder.dependencyArtifact("lunasvg", "bindings"));

    b.installArtifact(exe);
    b.installArtifact(try builder.dependencyArtifact("zbgfx", "shaderc"));

    const install_shaders = b.addInstallDirectory(.{
        .install_dir = .bin,
        .install_subdir = "assets/shaders/include",
        .source_dir = try builder.dependencyPath("zbgfx", "shaders"),
    });
    exe.step.dependOn(&install_shaders.step);

    const app_assets = b.addInstallDirectory(.{
        .install_dir = .bin,
        .install_subdir = "assets",
        .source_dir = b.path("assets"),
    });
    exe.step.dependOn(&app_assets.step);

    // Create the checker for 'zls'.
    {
        const exe_check = b.addExecutable(.{
            .name = "LevelSketch",
            .root_module = root_module,
        });
        builder.importAll(exe_check);

        if (builder.target.result.os.tag != .emscripten) {
            exe_check.linkLibrary(try builder.dependencyArtifact("zglfw", "glfw"));
            exe_check.linkLibrary(try builder.dependencyArtifact("lunasvg", "bindings"));
        }

        exe_check.linkLibrary(try builder.dependencyArtifact("zbgfx", "bgfx"));

        const check = b.step("check", "Check compile");
        check.dependOn(&exe_check.step);
    }

    const run_step = b.step("run", "Run the app");
    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    try builder.addTests(b, exe, &.{
        .{ .module_name = "core", .link_params = &.{} },
        .{
            .module_name = "io",
            .link_params = &.{.{ .dependency = "lunasvg", .artifact = "bindings" }},
        },
        .{ .module_name = "render", .link_params = &.{} },
        .{ .module_name = "world", .link_params = &.{} },
    });
}

const Builder = struct {
    const Self = @This();

    const Error = error{
        DependencyDoesntExist,
        ModuleDoesntExist,
    };

    const Dependency = struct {
        root: []const u8,
        dependency: *std.Build.Dependency,
    };

    const DependencyMap = std.StringHashMap(Dependency);
    const ModuleMap = std.StringHashMap(*std.Build.Module);

    build: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    dependencies: DependencyMap,
    modules: ModuleMap,

    pub fn init(allocator: std.mem.Allocator, build_in: *std.Build) Self {
        return Self{
            .build = build_in,
            .target = build_in.standardTargetOptions(.{}),
            .optimize = build_in.standardOptimizeOption(.{}),
            .dependencies = DependencyMap.init(allocator),
            .modules = ModuleMap.init(allocator),
        };
    }

    pub fn deinit(self: *Self) void {
        self.dependencies.deinit();
        self.modules.deinit();
    }

    pub fn addDependency(self: *Self, name: []const u8, root: []const u8) !void {
        const dependency = self.build.dependency(name, .{
            .target = self.target,
            .optimize = self.optimize,
        });
        try self.dependencies.put(name, .{
            .root = root,
            .dependency = dependency,
        });
    }

    pub fn addModule(
        self: *Self,
        name: []const u8,
        root: []const u8,
        imports: []const []const u8,
    ) !void {
        var module = self.build.createModule(.{
            .target = self.target,
            .optimize = self.optimize,
            .root_source_file = self.build.path(root),
        });

        for (imports) |import| {
            if (self.dependencies.get(import)) |dep| {
                module.addImport(import, dep.dependency.module(dep.root));
            } else if (self.modules.get(import)) |mod| {
                module.addImport(import, mod);
            } else {
                return Error.DependencyDoesntExist;
            }
        }

        try self.modules.put(name, module);
    }

    pub fn getModule(self: Self, name: []const u8) !*std.Build.Module {
        if (self.modules.get(name)) |module| {
            return module;
        }

        return Error.ModuleDoesntExist;
    }

    pub fn importAll(self: Self, compile: *std.Build.Step.Compile) void {
        var dependencies = self.dependencies.iterator();
        while (dependencies.next()) |entry| {
            const info = entry.value_ptr;
            compile.root_module.addImport(entry.key_ptr.*, info.dependency.module(info.root));
        }

        var modules = self.modules.iterator();
        while (modules.next()) |entry| {
            compile.root_module.addImport(entry.key_ptr.*, entry.value_ptr.*);
        }
    }

    pub fn dependencyArtifact(
        self: Self,
        name: []const u8,
        artifact: []const u8,
    ) !*std.Build.Step.Compile {
        if (self.dependencies.get(name)) |dependency| {
            return dependency.dependency.artifact(artifact);
        }

        return Error.DependencyDoesntExist;
    }

    pub fn dependencyPath(self: Self, name: []const u8, path: []const u8) !std.Build.LazyPath {
        if (self.dependencies.get(name)) |dependency| {
            return dependency.dependency.path(path);
        }

        return Error.DependencyDoesntExist;
    }

    pub const TestParam = struct {
        pub const LinkParam = struct {
            dependency: []const u8,
            artifact: []const u8,
        };

        module_name: []const u8,
        link_params: []const LinkParam,
    };

    pub fn addTests(
        self: Self,
        b: *std.Build,
        exe: *std.Build.Step.Compile,
        modules: []const TestParam,
    ) !void {
        const step = b.step("test", "Run tests");

        const exe_test = b.addTest(.{
            .root_module = exe.root_module,
        });
        const exe_run = b.addRunArtifact(exe_test);
        step.dependOn(&exe_run.step);

        for (modules) |param| {
            const module = try self.getModule(param.module_name);

            const module_test = b.addTest(.{
                .root_module = module,
            });

            for (param.link_params) |link| {
                const artifact = try self.dependencyArtifact(link.dependency, link.artifact);
                module_test.linkLibrary(artifact);
            }

            const run = b.addRunArtifact(module_test);

            step.dependOn(&run.step);
        }
    }
};
