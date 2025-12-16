const builtin = @import("builtin");
const core = @import("core");
const gui = @import("gui");
const std = @import("std");
const zglfw = @import("zglfw");

const Vec2u = core.math.Vec2u;

const Cursor = gui.Cursor;

pub const Window = struct {
    const Self = @This();

    handle: *zglfw.Window,
    cursor: Cursor = .{},

    pub fn init(handle: *zglfw.Window) Self {
        return .{
            .handle = handle,
        };
    }

    pub fn deinit(self: *Self) void {
        zglfw.destroyWindow(self.handle);
    }

    pub fn update(self: *Self) void {
        const cursor_pos = self.handle.getCursorPos();
        self.cursor.update(@intFromFloat(cursor_pos[0]), @intFromFloat(cursor_pos[1]));

        const left = zglfw.getMouseButton(self.handle, .left);
        const middle = zglfw.getMouseButton(self.handle, .middle);
        const right = zglfw.getMouseButton(self.handle, .right);

        self.updateCursorButton(.left, left);
        self.updateCursorButton(.middle, middle);
        self.updateCursorButton(.right, right);
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

    pub fn isPressed(self: Self, key: zglfw.Key) bool {
        const action = self.handle.getKey(key);
        return action == .press;
    }

    fn updateCursorButton(self: *Self, button: zglfw.MouseButton, action: zglfw.Action) void {
        const cursor_button = switch (button) {
            zglfw.MouseButton.left => Cursor.Button.left,
            zglfw.MouseButton.middle => Cursor.Button.middle,
            zglfw.MouseButton.right => Cursor.Button.right,
            else => Cursor.Button.unhandled,
        };

        const cursor_action = switch (action) {
            zglfw.Action.press => Cursor.Action.press,
            zglfw.Action.release => Cursor.Action.release,
            zglfw.Action.repeat => Cursor.Action.repeat,
        };

        self.cursor.buttons[@intFromEnum(cursor_button)] = cursor_action;
    }
};

pub var primary_window: Window = undefined;

pub fn init() !void {
    try zglfw.init();

    const zglfw_version = zglfw.getVersion();
    std.log.info("glfw version is {}.{}.{}", .{
        zglfw_version.major,
        zglfw_version.minor,
        zglfw_version.patch,
    });

    zglfw.windowHint(.client_api, .no_api);
    const handle = try zglfw.createWindow(960, 540, "Level Sketch", null);
    primary_window = .init(handle);
}

pub fn deinit() void {
    primary_window.deinit();
    zglfw.terminate();
}

pub fn update() void {
    zglfw.pollEvents();
    primary_window.update();
}
