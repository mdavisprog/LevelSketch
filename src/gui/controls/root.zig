const clay = @import("clay");

pub const buttons = @import("buttons.zig");
pub const checkbox = @import("checkbox.zig");
pub const images = @import("images.zig");
pub const panels = @import("panels.zig");

pub fn dummy(width: f32, height: f32) void {
    clay.openElement();
    clay.configureOpenElement(.{
        .layout = .{
            .sizing = .fixed(width, height),
        },
    });
    clay.closeElement();
}
