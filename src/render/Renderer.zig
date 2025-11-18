const core = @import("core");
const render = @import("root.zig");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Vec2f = core.math.Vec2f;

const Fonts = render.Fonts;
const Programs = render.shaders.Programs;
const MemFactory = render.MemFactory;
const Textures = render.Textures;

const Self = @This();

pub const world_state = zbgfx.bgfx.StateFlags_WriteRgb |
    zbgfx.bgfx.StateFlags_WriteA |
    zbgfx.bgfx.StateFlags_WriteZ |
    zbgfx.bgfx.StateFlags_DepthTestLess |
    zbgfx.bgfx.StateFlags_CullCw |
    zbgfx.bgfx.StateFlags_Msaa;

pub const ui_state = zbgfx.bgfx.StateFlags_WriteRgb |
    zbgfx.bgfx.StateFlags_WriteA |
    zbgfx.bgfx.StateFlags_Msaa |
    render.stateFlagsBlend(
        zbgfx.bgfx.StateFlags_BlendSrcAlpha,
        zbgfx.bgfx.StateFlags_BlendInvSrcAlpha,
    );

mem_factory: MemFactory,
textures: Textures,
programs: Programs,
fonts: *Fonts,
framebuffer_size: Vec2f = .zero,
_gpa: std.mem.Allocator,

pub fn init(gpa: std.mem.Allocator) !Self {
    var mem_factory = try MemFactory.init(gpa);
    const textures = try Textures.init(&mem_factory);
    const programs: Programs = .init(gpa);
    const fonts: *Fonts = try .init(gpa);
    return Self{
        .mem_factory = mem_factory,
        .textures = textures,
        .programs = programs,
        .fonts = fonts,
        ._gpa = gpa,
    };
}

pub fn deinit(self: *Self) void {
    self.fonts.*.deinit(self._gpa);
    self._gpa.destroy(self.fonts);

    self.programs.deinit(self._gpa);
    self.textures.deinit(self._gpa);
    self.mem_factory.deinit();
}

pub fn update(self: *Self) void {
    self.mem_factory.update();
}
