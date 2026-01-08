const app = @import("app");
const clay = @import("clay");
const ClayContext = @import("ClayContext.zig");
const ClayLayout = @import("ClayLayout.zig");
const core = @import("core");
const gui = @import("root.zig");
const render = @import("render");
const State = @import("State.zig");
const std = @import("std");
const _world = @import("world");

const Vec2f = core.math.Vec2f;

const Cursor = gui.Cursor;

const Commands = render.Commands;
const Font = render.Font;
const Program = render.shaders.Program;
const Renderer = render.Renderer;
const Texture = render.Texture;
const View = render.View;

const World = _world.World;

const Self = @This();

const id_panel: clay.ElementId = clay.idc("Panel");
const id_camera: clay.ElementId = clay.idc("ResetCamera");
const id_quit: clay.ElementId = clay.idc("QuitButton");

view: View,
font: *Font,
ui_shader: *Program,
text_shader: *Program,
clay_context: ClayContext,
_clay_layout: ClayLayout,
_commands: Commands,
_state: State,

/// Must be initialized after bgfx has been initialized.
pub fn init(renderer: *Renderer) !Self {
    const framebuffer_size = renderer.framebuffer_size;

    var view: View = .init(0x000000FF, false);
    view.setMode(.Sequential);
    view.setOrthographic(framebuffer_size.x, framebuffer_size.y);

    const font = try renderer.fonts.loadFile(
        renderer,
        "Roboto-Regular.ttf",
        48.0,
        .sdf,
    );

    const ui_shader: *Program = try renderer.programs.build(
        renderer.allocator,
        "ui",
        .{
            .varying_file_name = "ui/def.sc",
            .fragment_file_name = "ui/fragment.sc",
            .vertex_file_name = "ui/vertex.sc",
        },
    );

    const text_shader: *Program = try renderer.programs.build(
        renderer.allocator,
        "text",
        .{
            .varying_file_name = "ui/def.sc",
            .fragment_file_name = "ui/text_sdf.sc",
            .vertex_file_name = "ui/vertex.sc",
        },
    );

    const clay_context: ClayContext = try .init(renderer.allocator);

    const state: State = .init(renderer.allocator, font);

    var result = Self{
        .view = view,
        .font = font,
        .ui_shader = ui_shader,
        .text_shader = text_shader,
        ._commands = try Commands.init(renderer.mem_factory.allocator),
        .clay_context = clay_context,
        ._clay_layout = .init(renderer),
        ._state = state,
    };

    try result.layout();

    return result;
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self._state.deinit();
    self.clay_context.deinit(allocator);
    self._commands.deinit();
}

pub fn update(
    self: *Self,
    renderer: *Renderer,
    world: *World,
    delta_time: f32,
    cursor: Cursor,
) !void {
    _ = delta_time;

    const delta = cursor.delta();
    if (!delta.isZero() or cursor.didChange(.left)) {
        self._state.update(cursor, self, renderer, world);

        const position = cursor.current.toVec2f();
        clay.setPointerState(.init(position.x, position.y), cursor.pressed(.left));
        try self.layout();
    }
}

pub fn draw(self: *Self, renderer: *const Renderer) !void {
    const width: u16 = @intFromFloat(renderer.framebuffer_size.x);
    const height: u16 = @intFromFloat(renderer.framebuffer_size.y);
    self.view.submitOrthographic(width, height);

    try self._commands.run(self.view);
}

fn layout(self: *Self) !void {
    self._commands.clear();

    self._clay_layout.begin();
    {
        gui.controls.panels.begin(&self._state, id_panel, "Panel");
        {
            gui.controls.buttons.label(
                &self._state,
                id_camera,
                "Reset Camera",
                onResetCamera,
            );

            gui.controls.buttons.label(
                &self._state,
                id_quit,
                "Quit",
                onQuit,
            );
        }
        gui.controls.panels.end();
    }
    try self._clay_layout.end(&self._commands);
}

fn onResetCamera(context: State.Context) bool {
    context.world.camera.position = .init(0.0, 0.0, -3.0, 1.0);
    context.world.camera.resetRotation();
    return true;
}

fn onQuit(_: State.Context) bool {
    app.State.should_exit = true;
    return true;
}
