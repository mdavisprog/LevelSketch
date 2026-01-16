const std = @import("std");
const bgfx_import = @cImport({
    @cInclude("stdint.h");
    @cInclude("include/bgfx/defines.h");
    @cInclude("src/version.h");
});

pub const bgfx = @import("bgfx.zig");
pub const build = @import("build_step.zig");
pub const callbacks = @import("callbacks.zig");
pub const shaderc = @import("shaderc.zig");

pub const debugdraw = @import("debugdraw.zig");
pub const imgui_backend = @import("backend_bgfx.zig");

pub const version = struct {
    pub const api = bgfx_import.BGFX_API_VERSION;
    pub const rev = bgfx_import.BGFX_REV_NUMBER;
};
