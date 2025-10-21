const std = @import("std");
const version = @import("version");
const zglfw = @import("zglfw");

pub fn main() !void {
    std.log.info("Welcome to LevelSketch!", .{});
    std.log.info("Version is {}.{}.{}", .{
        version.major,
        version.minor,
        version.patch,
    });

    try zglfw.init();
    defer zglfw.terminate();

    const window = try zglfw.createWindow(960, 540, "Level Sketch", null);
    defer zglfw.destroyWindow(window);

    while (!window.shouldClose()) {
        zglfw.pollEvents();

        window.swapBuffers();
    }
}
