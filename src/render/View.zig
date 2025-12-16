const core = @import("core");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Mat = core.math.Mat;

const Self = @This();

var view_count: zbgfx.bgfx.ViewId = 0;

id: zbgfx.bgfx.ViewId,
clear_color: u32,
clear_depth: bool,
projection: Mat,

pub fn init(clear_color: u32, clear_depth: bool) Self {
    const id = view_count;
    view_count += 1;
    return Self{
        .id = id,
        .clear_color = clear_color,
        .clear_depth = clear_depth,
        .projection = .identity,
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

pub fn setPerspective(self: *Self, fov: f32, aspect: f32) void {
    self.projection = .initPerspectiveFovLh(fov, aspect);
}

pub fn setOrthographic(self: *Self, width: f32, height: f32) void {
    self.projection = .initOrthographicOffCenterLh(width, height);
}

pub fn setMode(self: Self, mode: zbgfx.bgfx.ViewMode) void {
    zbgfx.bgfx.setViewMode(self.id, mode);
}

pub fn submitPerspective(self: Self, perspective: Mat, width: u16, height: u16) void {
    self.submit(perspective, width, height);
}

pub fn submitOrthographic(self: Self, width: u16, height: u16) void {
    self.submit(.identity, width, height);
}

fn submit(self: Self, view_matrix: Mat, width: u16, height: u16) void {
    zbgfx.bgfx.setViewTransform(
        self.id,
        &view_matrix.toArray(),
        &self.projection.toArray(),
    );

    zbgfx.bgfx.setViewRect(self.id, 0, 0, width, height);
}
