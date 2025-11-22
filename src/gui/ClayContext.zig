const clay = @import("clay");
const std = @import("std");

const Self = @This();

_memory: ?[]const u8 = null,
_arena: clay.Arena = .{},
_context: ?*clay.Context = null,

pub fn init(allocator: std.mem.Allocator) !Self {
    const min_size = clay.minMemorySize();
    const memory = try allocator.alloc(u8, min_size);

    const bytes: f32 = @floatFromInt(min_size);
    const mb: f32 = bytes / 1024.0 / 1024.0;

    const arena = clay.createArenaWithCapacityAndMemory(
        min_size,
        @ptrCast(memory.ptr),
    );

    std.log.info("Clay arena memory size: {d:.2}MB", .{mb});

    const context = clay.initialize(arena, .{}, .{
        .error_handler_function = onError,
    });

    return Self{
        ._memory = memory,
        ._arena = arena,
        ._context = context,
    };
}

pub fn deinit(self: *Self, gpa: std.mem.Allocator) void {
    if (self._memory) |memory| {
        gpa.free(memory);
        self._memory = null;
    }
}

fn onError(error_data: clay.ErrorData) callconv(.c) void {
    std.log.warn(
        "Clay Error: {} Message: {s}",
        .{ error_data.error_type, error_data.error_text.str() },
    );
}
