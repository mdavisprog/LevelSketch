const builtin = @import("builtin");
const core = @import("core");
const glfw = @import("glfw.zig");
const platform = @import("root.zig");
const std = @import("std");
const zglfw = @import("zglfw");

const Action = platform.input.Action;
const Key = platform.input.Keyboard.Key;
const StaticArrayList = core.containers.StaticArrayList;
const Vec2u = core.math.Vec2u;

const Self = @This();

pub const KeyEvent = struct {
    key: Key = .unknown,
    action: Action = .release,
};
pub const KeyEventBuffer = StaticArrayList(KeyEvent, 8);

handle: *zglfw.Window,

pub fn init(allocator: std.mem.Allocator, title: []const u8, width: u32, height: u32) !Self {
    zglfw.windowHint(.client_api, .no_api);
    const handle = try zglfw.createWindow(@intCast(width), @intCast(height), @ptrCast(title), null);
    registerCallbacks(handle);

    const data = try allocator.create(Data);
    data.* = .{};
    handle.setUserPointer(data);

    return .{
        .handle = handle,
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    unregisterCallbacks(self.handle);

    if (self.handle.getUserPointer(Data)) |data| {
        allocator.destroy(data);
    }

    zglfw.destroyWindow(self.handle);
}

pub fn shouldClose(self: Self) bool {
    return self.handle.shouldClose();
}

pub fn nativeHandle(self: Self) ?*anyopaque {
    return switch (builtin.os.tag) {
        .windows => zglfw.getWin32Window(self.handle),
        else => {
            @compileError(std.fmt.comptimePrint(
                "Platform {} not currently handled.",
                .{builtin.os.tag},
            ));
        },
    };
}

pub fn toggleCursor(self: Self, enabled: bool) !void {
    try zglfw.setInputMode(self.handle, .cursor, if (enabled) .normal else .disabled);
    if (zglfw.rawMouseMotionSupported()) {
        try zglfw.setInputMode(self.handle, .raw_mouse_motion, enabled);
    }
}

pub fn framebufferSize(self: Self) Vec2u {
    const size = self.handle.getFramebufferSize();
    return .init(
        @intCast(size[0]),
        @intCast(size[1]),
    );
}

pub fn getKeyEvents(self: Self) *KeyEventBuffer {
    const data = self.handle.getUserPointer(Data).?;
    return &data.key_events;
}

fn registerCallbacks(window: *zglfw.Window) void {
    _ = zglfw.setKeyCallback(window, onKey);
}

fn unregisterCallbacks(window: *zglfw.Window) void {
    _ = zglfw.setKeyCallback(window, null);
}

const Data = struct {
    key_events: KeyEventBuffer = .empty,
};

fn onKey(
    window: *zglfw.Window,
    key: zglfw.Key,
    scancode: c_int,
    action: zglfw.Action,
    mods: zglfw.Mods,
) callconv(.c) void {
    _ = scancode;
    _ = mods;

    const data: *Data = window.getUserPointer(Data) orelse unreachable;
    const _key = glfw.toKey(key);
    const _action = glfw.toAction(action);

    data.key_events.append(.{
        .key = _key,
        .action = _action,
    }) catch |err| {
        std.debug.panic("Failed to append key event. Error: {}", .{err});
    };
}
