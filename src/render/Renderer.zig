const render = @import("root.zig");
const std = @import("std");

const MemFactory = render.MemFactory;
const Textures = render.Textures;

const Self = @This();

mem_factory: MemFactory,
textures: Textures,
_gpa: std.mem.Allocator,

pub fn init(gpa: std.mem.Allocator) !Self {
    var mem_factory = try MemFactory.init(gpa);
    const textures = try Textures.init(&mem_factory);
    return Self{
        .mem_factory = mem_factory,
        .textures = textures,
        ._gpa = gpa,
    };
}

pub fn deinit(self: *Self) void {
    self.textures.deinit(self._gpa);
    self.mem_factory.deinit();
}

pub fn update(self: *Self) void {
    self.mem_factory.update();
}
