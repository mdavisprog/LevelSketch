const std = @import("std");

pub const bgfx = @import("bgfx.zig");
pub const build = @import("build_step.zig");
pub const callbacks = @import("callbacks.zig");
pub const shaderc = @import("shaderc.zig");

pub const debugdraw = @import("debugdraw.zig");
pub const imgui_backend = @import("backend_bgfx.zig");

/// TODO: Try to pull this value directly from the header file.
pub const API_VERSION = 129;
pub const REV_VERSION = 8889;
