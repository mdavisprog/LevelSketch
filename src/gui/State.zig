const clay = @import("clay");
const core = @import("core");
const gui = @import("root.zig");
const platform = @import("platform");
const render = @import("render");
const std = @import("std");
const _world = @import("world");

const Font = render.Font;
const GUI = gui.GUI;
const Mouse = platform.input.Mouse;
const Rectf = core.math.Rectf;
const Renderer = render.Renderer;
const Theme = gui.Theme;
const Vec2f = core.math.Vec2f;
const World = _world.World;

const Self = @This();

/// Passed to each callback.
/// TODO: Reduce amount of objects passed in to the callback.
pub const Context = struct {
    element: clay.ElementId = .{},
    gui: *const GUI,
    world: *World,
    state: *Self,

    pub fn getDataMut(self: Context) ?*gui.controls.Data {
        return self.state.getDataMut(self.element);
    }
};

/// Callback function prototype for all mouse button interactions.
pub const OnPointerEvent = *const fn (context: Context) bool;
pub const OnPointerMove = *const fn (delta: Vec2f, context: Context) void;

/// Registered callbacks for an element.
pub const Callbacks = struct {
    on_press: ?OnPointerEvent = null,
    on_release: ?OnPointerEvent = null,
    on_click: ?OnPointerEvent = null,
    on_drag: ?OnPointerMove = null,
};

pub const DataMap = std.AutoHashMap(clay.ElementId, gui.controls.Data);

theme: Theme,
pressed: clay.ElementId = .{},
_callbacks: std.AutoHashMap(clay.ElementId, Callbacks),
_data_map: DataMap,

pub fn init(renderer: *Renderer, font: *const Font) !Self {
    return .{
        ._callbacks = .init(renderer.allocator),
        ._data_map = .init(renderer.allocator),
        .theme = try .init(renderer, font),
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self._callbacks.deinit();
    self._data_map.deinit();
    self.theme.deinit(allocator);
}

pub fn update(
    self: *Self,
    mouse: Mouse,
    _gui: *const GUI,
    world: *World,
) void {
    const hovered = clay.getPointerOverIds();
    if (hovered.length == 0) {
        return;
    }

    var context: Context = .{
        .gui = _gui,
        .state = self,
        .world = world,
    };

    // 'hovered' should be in order from topmost to bottom. Loop through each one and find the
    // first element that is registered.
    var pressed: clay.ElementId = if (mouse.justReleased(.left)) .{} else self.pressed;
    for (0..hovered.len()) |i| {
        const id = hovered.get(i);

        // If an element has callbacks, then consider it as a target element.
        if (self._callbacks.get(id)) |callbacks| {
            context.element = id;
            if (mouse.justPressed(.left)) {
                // If any button is pressed, record the element.
                if (pressed.id == 0) {
                    pressed = id;
                }

                if (callbacks.on_press) |on_press| {
                    if (on_press(context)) {
                        break;
                    }
                }
            }

            if (mouse.justReleased(.left)) {
                if (callbacks.on_release) |on_release| {
                    _ = on_release(context);
                }

                if (id.eql(self.pressed)) {
                    if (callbacks.on_click) |on_click| {
                        _ = on_click(context);
                    }
                }
            }
        }
    }

    if (!self.pressed.eql(pressed)) {
        self.pressed = pressed;
    }

    // Drag is performed here to avoid hover checks.
    if (self._callbacks.get(self.pressed)) |callbacks| {
        if (mouse.pressed(.left)) {
            if (callbacks.on_drag) |on_drag| {
                context.element = self.pressed;
                _ = on_drag(mouse.delta(), context);
            }
        }
    }
}

pub fn register(self: *Self, id: []const u8, callbacks: Callbacks) clay.ElementId {
    const result = clay.id(id);
    self.registerId(result, callbacks);
    return result;
}

pub fn registerId(self: *Self, id: clay.ElementId, callbacks: Callbacks) void {
    self._callbacks.put(id, callbacks) catch |err| {
        std.log.warn("Failed to allocate callbacks! Error: {}", .{err});
    };
}

/// Updates or adds the callbacks for the given element.
pub fn updateId(self: *Self, id: clay.ElementId, callbacks: Callbacks) void {
    if (self._callbacks.getPtr(id)) |value| {
        value.on_press = callbacks.on_press orelse value.on_press;
        value.on_release = callbacks.on_release orelse value.on_release;
        value.on_click = callbacks.on_click orelse value.on_click;
        value.on_drag = callbacks.on_drag orelse value.on_drag;
    } else {
        self._callbacks.put(id, callbacks) catch |err| {
            std.log.warn("Failed to update callbacks! Error: {}", .{err});
        };
    }
}

pub fn setData(self: *Self, id: clay.ElementId, data: gui.controls.Data) void {
    self._data_map.put(id, data) catch |err| {
        std.debug.panic("Failed to set data for element '{s}'. Err: {}", .{
            id.string_id.str(),
            err,
        });
    };
}

pub fn getOrSetData(self: *Self, id: clay.ElementId, data: gui.controls.Data) gui.controls.Data {
    if (self._data_map.get(id)) |entry| {
        return entry;
    }

    self._data_map.put(id, data) catch |err| {
        std.debug.panic("Failed to set data for element '{s}'. Err: {}", .{
            id.string_id.str(),
            err,
        });
    };

    return self._data_map.get(id) orelse {
        std.debug.panic("Failed to return set data for element '{s}'.", .{id.string_id.str()});
    };
}

pub fn getDataMut(self: Self, id: clay.ElementId) ?*gui.controls.Data {
    return self._data_map.getPtr(id);
}
