const clay = @import("clay");
const core = @import("core");
const gui = @import("root.zig");
const render = @import("render");
const std = @import("std");
const _world = @import("world");

const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;

const Cursor = gui.Cursor;
const GUI = gui.GUI;
const Theme = gui.Theme;

const Font = render.Font;
const Renderer = render.Renderer;

const World = _world.World;

const Self = @This();

/// Passed to each callback.
/// TODO: Reduce amount of objects passed in to the callback.
pub const Context = struct {
    element: clay.ElementId = .{},
    gui: *const GUI,
    renderer: *const Renderer,
    world: *World,
    state: *Self,
};

pub const DataType = enum {
    boolean,
    rect,
};

pub const Data = union(DataType) {
    boolean: bool,
    rect: Rectf,

    fn init() Data {
        return .{
            .rect = .zero,
        };
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

theme: Theme,
pressed: clay.ElementId = .{},
_callbacks: std.AutoHashMap(clay.ElementId, Callbacks),
_data: std.AutoHashMap(clay.ElementId, Data),

pub fn init(renderer: *Renderer, font: *const Font) !Self {
    return .{
        ._callbacks = .init(renderer.allocator),
        ._data = .init(renderer.allocator),
        .theme = try .init(renderer, font),
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self._callbacks.deinit();
    self._data.deinit();
    self.theme.deinit(allocator);
}

pub fn update(
    self: *Self,
    cursor: Cursor,
    _gui: *const GUI,
    renderer: *const Renderer,
    world: *World,
) void {
    const hovered = clay.getPointerOverIds();
    if (hovered.length == 0) {
        return;
    }

    var context: Context = .{
        .gui = _gui,
        .renderer = renderer,
        .state = self,
        .world = world,
    };

    // 'hovered' should be in order from topmost to bottom. Loop through each one and find the
    // first element that is registered.
    var pressed: clay.ElementId = if (cursor.justReleased(.left)) .{} else self.pressed;
    for (0..hovered.len()) |i| {
        const id = hovered.get(i);

        // If an element has callbacks, then consider it as a target element.
        if (self._callbacks.get(id)) |callbacks| {
            context.element = id;
            if (cursor.justPressed(.left)) {
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

            if (cursor.justReleased(.left)) {
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
        if (cursor.pressed(.left)) {
            if (callbacks.on_drag) |on_drag| {
                context.element = self.pressed;
                _ = on_drag(cursor.delta().toVec2f(), context);
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

pub fn registerData(self: *Self, id: clay.ElementId, data: anytype) @TypeOf(data) {
    const new = !self._data.contains(id);
    const entry: *Data = blk: {
        if (!self._data.contains(id)) {
            self._data.put(id, .init()) catch |err| {
                std.debug.panic("Failed to allocate memory for data. Error: {}", .{err});
            };
        }

        break :blk self._data.getPtr(id).?;
    };

    if (new) {
        updateDataEntry(entry, data);
    }

    return self.getData(@TypeOf(data), id) orelse std.debug.panic("Failed to register data!", .{});
}

pub fn updateData(self: *Self, id: clay.ElementId, data: anytype) void {
    if (self._data.getPtr(id)) |entry| {
        updateDataEntry(entry, data);
    }
}

/// Making self mutable since data within the map will be modified.
pub fn getDataMut(self: *Self, id: clay.ElementId) ?*Data {
    return self._data.getPtr(id);
}

pub fn getData(self: Self, comptime T: type, id: clay.ElementId) ?T {
    if (self._data.get(id)) |data| {
        switch (T) {
            bool => {
                std.debug.assert(data == .boolean);
                return data.boolean;
            },
            Rectf => {
                std.debug.assert(data == .rect);
                return data.rect;
            },
            else => {},
        }
    }

    return null;
}

fn updateDataEntry(entry: *Data, data: anytype) void {
    const Type = @TypeOf(data);
    switch (Type) {
        bool => entry.* = .{ .boolean = data },
        Rectf => entry.* = .{ .rect = data },
        else => {
            @compileError(std.fmt.comptimePrint(
                "Data is an invalid type: {s}!",
                .{@typeName(Type)},
            ));
        },
    }
}
