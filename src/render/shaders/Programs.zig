const shaders = @import("root.zig");
const std = @import("std");

const Program = shaders.Program;

const Self = @This();

pub const Error = error{
    ProgramAlreadyExists,
    ProgramNotFound,
};

const ProgramMap = std.StringHashMap(*Program);

programs: ProgramMap,

pub fn init(gpa: std.mem.Allocator) Self {
    const programs = ProgramMap.init(gpa);
    return Self{
        .programs = programs,
    };
}

pub fn deinit(self: *Self, gpa: std.mem.Allocator) void {
    var it = self.programs.iterator();
    while (it.next()) |item| {
        gpa.free(item.key_ptr.*);

        item.value_ptr.*.deinit();
        gpa.destroy(item.value_ptr.*);
    }

    self.programs.deinit();
}

pub fn build(self: *Self, gpa: std.mem.Allocator, name: []const u8, paths: Program.Paths) !*Program {
    if (self.programs.contains(name)) {
        return Error.ProgramAlreadyExists;
    }

    const program: *Program = try .init(gpa);
    try program.*.build(paths);

    try self.programs.put(try gpa.dupe(u8, name), program);

    return program;
}

pub fn get(self: Self, name: []const u8) !*Program {
    if (self.programs.get(name)) |program| {
        return program;
    }

    return Error.ProgramNotFound;
}
