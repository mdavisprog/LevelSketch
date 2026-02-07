const std = @import("std");

const Blake3 = std.crypto.hash.Blake3;

/// Hashes a struct to generate a unique u32.
/// TODO: Better version to uniquely identify types.
pub fn hashStruct(comptime T: type) u32 {
    const info = @typeInfo(T);
    if (info != .@"struct") {
        @compileError(std.fmt.comptimePrint("Given event type {} is not a struct.", .{T}));
    }

    var hasher: Blake3 = .init(.{});
    hasher.update(@typeName(T));

    inline for(info.@"struct".fields) |field| {
        hasher.update(field.name);
        hasher.update(@typeName(field.type));
    }

    var buffer: [4]u8 = undefined;
    hasher.final(&buffer);

    return std.mem.bytesToValue(u32, &buffer);
}
