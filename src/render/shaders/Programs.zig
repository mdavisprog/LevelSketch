const shaders = @import("root.zig");
const std = @import("std");

const Program = shaders.Program;

const Self = @This();

pub const Error = error{
    ProgramAlreadyExists,
};

const ProgramMap = std.AutoHashMapUnmanaged(Program.Handle, *Program);
const ProgramNameMap = std.StringHashMapUnmanaged(Program.Handle);

programs: ProgramMap = .empty,
program_names: ProgramNameMap = .empty,

pub fn init() Self {
    return .{};
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    var names = self.program_names.keyIterator();
    while (names.next()) |name| {
        allocator.free(name.*);
    }
    self.program_names.deinit(allocator);

    var it = self.programs.iterator();
    while (it.next()) |entry| {
        entry.value_ptr.*.deinit();
        allocator.destroy(entry.value_ptr.*);
    }
    self.programs.deinit(allocator);
}

pub fn build(
    self: *Self,
    allocator: std.mem.Allocator,
    paths: Program.Paths,
) !Program.Handle {
    const program: *Program = try allocator.create(Program);
    program.* = .init(allocator);
    try program.build(paths);

    const handle: Program.Handle = .generate();
    try self.programs.put(allocator, handle, program);

    return handle;
}

pub fn buildWithName(
    self: *Self,
    allocator: std.mem.Allocator,
    name: []const u8,
    paths: Program.Paths,
) !Program.Handle {
    const program: *Program = try allocator.create(Program);
    program.* = .init(allocator);
    try program.build(paths);

    const handle: Program.Handle = .generate();
    try self.programs.put(allocator, handle, program);
    try self.program_names.put(allocator, try allocator.dupe(u8, name), handle);

    return handle;
}

pub fn get(self: Self, handle: Program.Handle) ?*Program {
    return self.programs.get(handle);
}

pub fn getByName(self: Self, name: []const u8) ?*Program {
    if (self.program_names.get(name)) |handle| {
        return self.programs.get(handle);
    }

    return null;
}
