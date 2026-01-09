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

pub const Type = enum {
    checkbox,
    panel,
};

pub const Data = union(Type) {
    checkbox: checkbox.Checkbox,
    panel: panels.Panel,

    pub fn init() Data {
        return .{
            .checkbox = .{},
        };
    }
};
