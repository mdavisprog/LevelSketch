const render = @import("root.zig");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Program = render.shaders.Program;
const RenderBuffer = render.RenderBuffer;
const Texture = render.Texture;
const Uniform = render.shaders.Uniform;
const View = render.View;

pub const Command = struct {
    buffer: RenderBuffer,
    texture: Texture,
    texture_flags: u32,
    shader: *Program,
    sampler: Uniform,
    state: u64,
};

const Self = @This();

_commands: std.ArrayList(Command),
_gpa: std.mem.Allocator,

pub fn init(gpa: std.mem.Allocator) !Self {
    const commands = try std.ArrayList(Command).initCapacity(gpa, 0);
    return Self{
        ._commands = commands,
        ._gpa = gpa,
    };
}

pub fn deinit(self: *Self) void {
    for (self._commands.items) |*command| {
        command.buffer.deinit();
    }

    self._commands.deinit(self._gpa);
}

pub fn addCommand(
    self: *Self,
    command: Command,
) !void {
    try self._commands.append(self._gpa, command);
}

pub fn clear(self: *Self) void {
    self._commands.clearAndFree(self._gpa);
}

pub fn run(
    self: *Self,
    view: View,
) !void {
    for (self._commands.items) |command| {
        try command.texture.bind(command.sampler.handle, command.texture_flags);
        command.buffer.bind(command.state);
        zbgfx.bgfx.submit(view.id, command.shader.handle.data, 255, 0);
    }

    view.touch();
}
