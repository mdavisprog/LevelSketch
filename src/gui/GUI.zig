const clay = @import("clay");
const ClayContext = @import("ClayContext.zig");
const ClayLayout = @import("ClayLayout.zig");
const core = @import("core");
const editor = @import("editor");
const gui = @import("root.zig");
const platform = @import("platform");
const render = @import("render");
const State = @import("State.zig");
const std = @import("std");
const _world = @import("world");

const Commands = render.Commands;
const Font = render.Font;
const Program = render.shaders.Program;
const Renderer = render.Renderer;
const Texture = render.Texture;
const Vec = core.math.Vec;
const Vec2f = core.math.Vec2f;
const View = render.View;
const World = _world.World;

const Self = @This();

const id_panel: clay.ElementId = clay.idc("Panel");
const id_camera: clay.ElementId = clay.idc("ResetCamera");
const id_quit: clay.ElementId = clay.idc("QuitButton");

view: View,
font: *Font,
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

    _ = try renderer.programs.buildWithName(
        renderer.allocator,
        "ui",
        .{
            .varying_file_name = "ui/def.sc",
            .fragment_file_name = "ui/fragment.sc",
            .vertex_file_name = "ui/vertex.sc",
        },
    );

    _ = try renderer.programs.buildWithName(
        renderer.allocator,
        "text",
        .{
            .varying_file_name = "ui/def.sc",
            .fragment_file_name = "ui/text_sdf.sc",
            .vertex_file_name = "ui/vertex.sc",
        },
    );

    const clay_context: ClayContext = try .init(renderer.allocator);

    const state: State = try .init(renderer, font);

    return .{
        .view = view,
        .font = font,
        ._commands = try Commands.init(renderer.mem_factory.allocator),
        .clay_context = clay_context,
        ._clay_layout = .init(renderer),
        ._state = state,
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self._state.deinit(allocator);
    self.clay_context.deinit(allocator);
    self._commands.deinit();
}

pub fn update(
    self: *Self,
    world: *World,
    mouse: platform.input.Mouse,
) !void {
    const delta = mouse.delta();
    if (!delta.isZero() or mouse.didChange(.left)) {
        self._state.update(mouse, self, world);

        const position = mouse.current;
        clay.setPointerState(.init(position.x, position.y), mouse.pressed(.left));
        try self.layout(world);
    }
}

pub fn draw(self: *Self, renderer: *const Renderer) !void {
    const width: u16 = @intFromFloat(renderer.framebuffer_size.x);
    const height: u16 = @intFromFloat(renderer.framebuffer_size.y);
    self.view.submitOrthographic(width, height);

    const common = renderer.programs.getByName("common").?;
    try common.setUniform("u_color", Vec.splat(1.0));

    try self._commands.run(self.view);
}

fn layout(self: *Self, world: *World) !void {
    const orbit_enabled = if (world.getResource(editor.resources.Orbit)) |orbit|
        orbit.enabled
    else
        false;

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

            gui.controls.checkbox.check(
                &self._state,
                clay.idc("LightOrbit"),
                "Orbit",
                orbit_enabled,
                onToggleOrbit,
            );
        }
        gui.controls.panels.end();
    }
    try self._clay_layout.end(&self._commands);
}

fn onResetCamera(context: State.Context) bool {
    context.world.triggerEvent(editor.events.ResetCamera{});
    return true;
}

fn onQuit(context: State.Context) bool {
    const _platform = context.world.getResource(platform.ecs.resources.Platform) orelse return false;
    _platform.primary_window.setShouldClose(true);
    return true;
}

fn onToggleOrbit(context: State.Context) bool {
    const data = context.getDataMut() orelse return false;
    const orbit = context.world.getResource(editor.resources.Orbit) orelse return false;
    orbit.enabled = data.checkbox.checked;
    return true;
}
