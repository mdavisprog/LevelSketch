const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const plutovg = b.addLibrary(.{
        .name = "plutovg",
        .linkage = .static,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    plutovg.addCSourceFiles(.{
        .files = &.{
            "lib/plutovg/source/plutovg-blend.c",
            "lib/plutovg/source/plutovg-canvas.c",
            "lib/plutovg/source/plutovg-font.c",
            "lib/plutovg/source/plutovg-ft-math.c",
            "lib/plutovg/source/plutovg-ft-raster.c",
            "lib/plutovg/source/plutovg-ft-stroker.c",
            "lib/plutovg/source/plutovg-matrix.c",
            "lib/plutovg/source/plutovg-paint.c",
            "lib/plutovg/source/plutovg-path.c",
            "lib/plutovg/source/plutovg-rasterize.c",
            "lib/plutovg/source/plutovg-surface.c",
        },
        .flags = &.{
            "-std=c11",
            "-DPLUTOVG_BUILD",
            "-DPLUTOVG_BUILD_STATIC",
        },
    });
    plutovg.root_module.addIncludePath(b.path("lib/plutovg/include/"));
    plutovg.linkLibC();

    const lunasvg = b.addLibrary(.{
        .name = "lunasvg",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    lunasvg.addCSourceFiles(.{
        .files = &.{
            "lib/source/graphics.cpp",
            "lib/source/lunasvg.cpp",
            "lib/source/svgelement.cpp",
            "lib/source/svggeometryelement.cpp",
            "lib/source/svglayoutstate.cpp",
            "lib/source/svgpaintelement.cpp",
            "lib/source/svgparser.cpp",
            "lib/source/svgproperty.cpp",
            "lib/source/svgrenderstate.cpp",
            "lib/source/svgtextelement.cpp",
        },
        .flags = &.{
            "-std=c++17",
            "-DLUNASVG_BUILD",
            "-DLUNASVG_BUILD_STATIC",
            "-DPLUTOVG_BUILD",
            "-DPLUTOVG_BUILD_STATIC",
        },
    });
    lunasvg.root_module.addIncludePath(b.path("lib/plutovg/include/"));
    lunasvg.root_module.addIncludePath(b.path("lib/include/"));
    lunasvg.linkLibrary(plutovg);
    lunasvg.linkLibCpp();

    const bindings = b.addLibrary(.{
        .name = "bindings",
        .linkage = .static,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    bindings.addCSourceFile(.{
        .file = b.path("src/bindings.cpp"),
        .flags = &.{
            "-std=c++17",
            "-DLUNASVG_BUILD",
            "-DLUNASVG_BUILD_STATIC",
            "-DPLUTOVG_BUILD",
            "-DPLUTOVG_BUILD_STATIC",
        },
    });
    bindings.root_module.addIncludePath(b.path("lib/include/"));
    bindings.linkLibrary(lunasvg);
    b.installArtifact(bindings);

    _ = b.addModule("root", .{
        .root_source_file = b.path("src/root.zig"),
        .target = target,
        .optimize = optimize,
    });
}
