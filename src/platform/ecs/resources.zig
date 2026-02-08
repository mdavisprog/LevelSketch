const Window = @import("../Window.zig");

pub const Frame = struct {
    pub const Times = struct {
        current: f64 = 0.0,
        elapsed: f64 = 0.0,
        delta: f32 = 0.0,
    };

    times: Times = .{},
    count: usize = 0,
};

pub const Platform = struct {
    primary_window: Window,
};
