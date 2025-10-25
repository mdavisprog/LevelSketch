const Camera = @import("Camera.zig");
const std = @import("std");
const zbgfx = @import("zbgfx");
const zmath = @import("zmath");

const Self = @This();

var view_count: zbgfx.bgfx.ViewId = 0;

id: zbgfx.bgfx.ViewId,
clear_color: u32,
clear_depth: bool,
projection: zmath.Mat,

pub fn init(clear_color: u32, clear_depth: bool) Self {
    const id = view_count;
    view_count += 1;
    return Self{
        .id = id,
        .clear_color = clear_color,
        .clear_depth = clear_depth,
        .projection = zmath.identity(),
    };
}

pub fn clear(self: Self) void {
    var flags: u16 = zbgfx.bgfx.ClearFlags_Color;
    if (self.clear_depth) {
        flags |= zbgfx.bgfx.ClearFlags_Depth;
    }

    zbgfx.bgfx.setViewClear(
        self.id,
        flags,
        self.clear_color,
        1.0,
        0,
    );
}

pub fn touch(self: Self) void {
    zbgfx.bgfx.touch(self.id);
}

pub fn setPerspective(self: *Self, fov: f32, aspect: f32) *Self {
    const fov_rad = std.math.degreesToRadians(fov);
    self.projection = zmath.perspectiveFovLh(fov_rad, aspect, 0.1, 100.0);
    return self;
}

pub fn set(self: Self, camera: Camera, width: u16, height: u16) void {
    const view_matrix = camera.toLookAt();
    zbgfx.bgfx.setViewTransform(
        self.id,
        &zmath.matToArr(view_matrix),
        &zmath.matToArr(self.projection),
    );

    zbgfx.bgfx.setViewRect(self.id, 0, 0, width, height);
}
