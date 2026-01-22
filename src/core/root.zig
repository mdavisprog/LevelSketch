pub const commandline = @import("commandline.zig");
pub const containers = @import("containers/root.zig");
pub const math = @import("math/root.zig");

const std = @import("std");

pub fn Handle(comptime T: type) type {
    return struct {
        const Self = @This();

        pub const Id = u32;
        pub const invalid_id: Id = std.math.maxInt(Id);
        pub const invalid: Self = .init(invalid_id);

        const Type = struct {
            const name = @typeName(T);
            var id: Id = 0;
        };

        id: Id = invalid_id,

        pub fn init(id: Id) Self {
            return .{
                .id = id,
            };
        }

        pub fn generate() Self {
            const id = Type.id;
            Type.id += 1;
            return .init(id);
        }

        pub fn eql(self: Self, other: Self) bool {
            return self.id == other.id;
        }

        pub fn isValid(self: Self) bool {
            return !self.eql(invalid);
        }

        pub fn typeName(_: Self) []const u8 {
            return Type.name;
        }
    };
}

test "refall" {
    std.testing.refAllDecls(@This());
}

test "handles" {
    const TestStruct1 = struct {};
    const TestStruct2 = struct {};

    const TestHandle1 = Handle(TestStruct1);
    const TestHandle2 = Handle(TestStruct2);

    const handle1: TestHandle1 = .generate();
    const handle2: TestHandle2 = .generate();

    try std.testing.expectEqual(0, handle1.id);
    try std.testing.expectEqual(0, handle2.id);
}
