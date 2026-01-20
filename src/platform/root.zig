const glfw = @import("glfw.zig");
const std = @import("std");
const Window = @import("Window.zig");
const world = @import("world");
const zglfw = @import("zglfw");

const SystemParam = world.Systems.SystemParam;
const World = world.World;

pub const input = @import("input/root.zig");

pub const resources = struct {
    pub const Platform = struct {
        primary_window: Window,
    };
};

pub fn init(_world: *World) !void {
    try zglfw.init();

    const zglfw_version = zglfw.getVersion();
    std.log.info("glfw version is {}.{}.{}", .{
        zglfw_version.major,
        zglfw_version.minor,
        zglfw_version.patch,
    });

    const window: Window = try .init(_world._allocator, "Level Sketch", 960, 540);
    try _world.registerResource(resources.Platform, .{
        .primary_window = window,
    });
    try _world.registerResource(input.resources.Keyboard, .{});
    try _world.registerResource(input.resources.Mouse, .{});

    _ = try _world.registerSystem(&.{}, .update, update);
    _ = try _world.registerSystem(&.{}, .shutdown, shutdown);
}

fn update(param: SystemParam) void {
    const allocator = param.world._allocator;

    const platform = param.world.getResource(resources.Platform) orelse unreachable;
    const keyboard = param.world.getResource(input.resources.Keyboard) orelse unreachable;
    const mouse = param.world.getResource(input.resources.Mouse) orelse unreachable;

    // Move any just_pressed keys to the pressed set. The release action will clear the
    // pressed events. Also want to clear the just_pressed/just_released sets. Those should
    // only be active for a single frame.
    keyboard.state.pressed.setFrom(allocator, keyboard.state.just_pressed) catch |err| {
        std.debug.panic("Failed to set pressed keys from just_pressed. Error: {}", .{err});
    };
    keyboard.state.just_pressed.clearRetainingCapacity();
    keyboard.state.just_released.clearRetainingCapacity();

    zglfw.pollEvents();

    const cursor_pos = platform.primary_window.handle.getCursorPos();
    mouse.state.update(@floatCast(cursor_pos[0]), @floatCast(cursor_pos[1]));

    const buttons: [5]zglfw.MouseButton = .{
        .left,
        .middle,
        .right,
        .four,
        .five,
    };

    for (buttons) |button| {
        const action = zglfw.getMouseButton(platform.primary_window.handle, button);
        const mapped = glfw.toMouseButton(button);
        mouse.state.buttons[@intFromEnum(mapped)] = glfw.toAction(action);
    }

    // Grab the stored key events from the primary window and update the Keyboard resource with
    // the key states.
    // TODO: Take into account multiple windows.
    const key_events = platform.primary_window.getKeyEvents();
    for (key_events.items) |event| {
        switch (event.action) {
            .press => {
                keyboard.state.just_pressed.insert(allocator, event.key) catch |err| {
                    std.debug.panic("Failed to add key to just_pressed set. Error: {}", .{err});
                };
            },
            .release => {
                _ = keyboard.state.just_pressed.remove(event.key);
                _ = keyboard.state.pressed.remove(event.key);
                keyboard.state.just_released.insert(allocator, event.key) catch |err| {
                    std.debug.panic("Failed to add key to just_released set. Error: {}", .{err});
                };
            },
            .repeat => {
                _ = keyboard.state.just_pressed.remove(event.key);
                keyboard.state.pressed.insert(allocator, event.key) catch |err| {
                    std.debug.panic("Failed to add key to pressed set. Error: {}", .{err});
                };
            },
        }
    }
    key_events.clear();
}

fn shutdown(param: SystemParam) void {
    const platform = param.world.getResource(resources.Platform) orelse unreachable;
    platform.primary_window.deinit(param.world._allocator);

    const keyboard = param.world.getResource(input.resources.Keyboard) orelse unreachable;
    keyboard.state.deinit(param.world._allocator);

    zglfw.terminate();
}
