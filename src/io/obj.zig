const core = @import("core");
const std = @import("std");

const Vec3f = core.math.Vec3f;
const Vec4f = core.math.Vec4f;

/// Represents a single model defined in an obj file.
pub const Model = struct {
    const Self = @This();

    pub const Face = struct {
        pub const Element = struct {
            vertex: u32 = 0,
            texture: u32 = 0,
            normal: u32 = 0,
        };

        elements: std.ArrayList(Element),

        pub fn init(allocator: std.mem.Allocator) !Face {
            return .{
                // Faces must at least have 3 elements to define a triangle.
                .elements = try .initCapacity(allocator, 3),
            };
        }

        pub fn deinit(self: *Face, allocator: std.mem.Allocator) void {
            self.elements.deinit(allocator);
        }

        pub fn get(self: Face, index: usize) ?Element {
            if (self.elements.items.len == 0 or index >= self.elements.items.len) {
                return null;
            }

            return self.elements.items[index];
        }

        pub fn getVertex(self: Face, index: usize) ?u32 {
            const element = self.get(index) orelse return null;
            return element.vertex -% 1;
        }

        pub fn getTexture(self: Face, index: usize) ?u32 {
            const element = self.get(index) orelse return null;
            return element.texture -% 1;
        }

        pub fn getNormal(self: Face, index: usize) ?u32 {
            const element = self.get(index) orelse return null;
            return element.normal -% 1;
        }
    };

    vertices: std.ArrayList(Vec4f),
    tex_coords: std.ArrayList(Vec3f),
    normals: std.ArrayList(Vec3f),
    faces: std.ArrayList(Face),

    pub fn init(allocator: std.mem.Allocator) !Self {
        return .{
            .vertices = try .initCapacity(allocator, 0),
            .tex_coords = try .initCapacity(allocator, 0),
            .normals = try .initCapacity(allocator, 0),
            .faces = try .initCapacity(allocator, 0),
        };
    }

    pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
        self.vertices.deinit(allocator);
        self.tex_coords.deinit(allocator);
        self.normals.deinit(allocator);

        for (self.faces.items) |*face| {
            face.*.deinit(allocator);
        }
        self.faces.deinit(allocator);
    }

    pub fn getFace(self: Self, face: usize) ?Face {
        if (self.faces.items.len == 0 or face >= self.faces.items.len) {
            return null;
        }

        return self.faces.items[face];
    }

    pub fn getVertex(self: Self, index: usize) ?Vec4f {
        if (self.vertices.items.len == 0 or index >= self.vertices.items.len) {
            return null;
        }

        return self.vertices.items[index];
    }

    pub fn getVertexFace(self: Self, face: usize, index: usize) ?Vec4f {
        const face_ = self.getFace(face) orelse return null;
        const element = face_.get(index) orelse return null;
        const vertex: usize = @intCast(element.vertex -% 1);
        return self.getVertex(vertex);
    }

    pub fn getTextureCoord(self: Self, index: usize) ?Vec3f {
        if (self.tex_coords.items.len == 0 or index >= self.tex_coords.items.len) {
            return null;
        }

        return self.tex_coords.items[index];
    }

    pub fn getTextureCoordFace(self: Self, face: usize, index: usize) ?Vec3f {
        const face_ = self.getFace(face) orelse return null;
        const element = face_.get(index) orelse return null;
        const tex_coord: usize = @intCast(element.texture -% 1);
        return self.getTextureCoord(tex_coord);
    }

    pub fn getNormal(self: Self, index: usize) ?Vec3f {
        if (self.normals.items.len == 0 or index >= self.tex_coords.items.len) {
            return null;
        }

        return self.normals.items[index];
    }

    pub fn getNormalFace(self: Self, face: usize, index: usize) ?Vec3f {
        const face_ = self.getFace(face) orelse return null;
        const element = face_.get(index) orelse return null;
        const normal: usize = @intCast(element.normal -% 1);
        return self.getNormal(normal);
    }
};

/// 'path' must be an absolute path to the asset.
pub fn loadFile(allocator: std.mem.Allocator, path: []const u8) !Model {
    var file = try std.fs.openFileAbsolute(path, .{});
    defer file.close();

    var buffer: [1024]u8 = undefined;
    var reader = file.reader(&buffer);

    return try processModel(allocator, &reader.interface);
}

pub fn loadData(allocator: std.mem.Allocator, data: []const u8) !Model {
    var reader: std.Io.Reader = .fixed(data);
    return try processModel(allocator, &reader);
}

fn processModel(allocator: std.mem.Allocator, reader: *std.Io.Reader) !Model {
    var result = try Model.init(allocator);

    while (true) {
        const line = reader.peekDelimiterExclusive('\n') catch |err| switch (err) {
            error.EndOfStream => {
                break;
            },
            else => {
                return err;
            },
        };

        try processLine(allocator, &result, line);
        reader.toss(std.mem.min(usize, &[2]usize{ line.len + 1, reader.bufferedLen() }));
    }

    return result;
}

fn processLine(
    allocator: std.mem.Allocator,
    model: *Model,
    line: []const u8,
) !void {
    if (line.len == 0) {
        return;
    }

    const element = line[0];
    switch (element) {
        '#' => {},
        'v' => {
            switch (line[1]) {
                // Ignore normal and texture coordinates for now.
                'n' => {
                    const normal = processVec3f(line);
                    try model.normals.append(allocator, normal);
                },
                't' => {
                    const texture_coord = processVec3f(line);
                    try model.tex_coords.append(allocator, texture_coord);
                },
                'p' => {
                    // Currently not supported.
                    // TODO: Emit a single warning if this element is found.
                },
                // If the next character is not one of the above, then assume
                // it is a vertex.
                else => {
                    const vertex = processVertex(line);
                    try model.vertices.append(allocator, vertex);
                },
            }
        },
        'f' => {
            const face = try processFace(allocator, line);
            try model.faces.append(allocator, face);
        },
        else => {},
    }
}

fn processVertex(line: []const u8) Vec4f {
    var it = std.mem.tokenizeAny(u8, line, " ");

    // Skip the 'v' character.
    _ = it.next();

    var result = Vec4f.zero;
    result.x = std.fmt.parseFloat(f32, it.next() orelse "") catch 0.0;
    result.y = std.fmt.parseFloat(f32, it.next() orelse "") catch 0.0;
    result.z = std.fmt.parseFloat(f32, it.next() orelse "") catch 0.0;
    result.w = std.fmt.parseFloat(f32, it.next() orelse "") catch 1.0;
    return result;
}

fn processVec3f(line: []const u8) Vec3f {
    var it = std.mem.tokenizeAny(u8, line, " ");

    // Skip the key character.
    _ = it.next();

    var result = Vec3f.zero;
    result.x = std.fmt.parseFloat(f32, it.next() orelse "") catch 0.0;
    result.y = std.fmt.parseFloat(f32, it.next() orelse "") catch 0.0;
    result.z = std.fmt.parseFloat(f32, it.next() orelse "") catch 0.0;
    return result;
}

fn processFace(allocator: std.mem.Allocator, line: []const u8) !Model.Face {
    var it = std.mem.tokenizeAny(u8, line, " ");

    // Skip the 'f' character.
    _ = it.next();

    var result: Model.Face = try .init(allocator);
    while (it.next()) |token| {
        const indices = processFaceIndices(token);
        try result.elements.append(allocator, .{
            .vertex = indices[0],
            .texture = indices[1],
            .normal = indices[2],
        });
    }

    return result;
}

/// Tokenize doesn't work here as some indices may be empty, which should default to a '0' value.
/// e.g. 1//1 means no texture coordinate associated with this vertex.
const FaceIndexParser = struct {
    const Self = @This();

    buffer: []const u8,
    index: usize = 0,

    fn next(self: *Self) ?u32 {
        const next_index = std.mem.indexOfPos(u8, self.buffer, self.index, "/");
        if (next_index) |index| {
            const len = index - self.index;
            const element = self.buffer[self.index..index];
            self.*.index += len + 1;

            return std.fmt.parseInt(u32, element, 10) catch 0;
        } else if (self.index < self.buffer.len) {
            const element = self.buffer[self.index..];
            self.*.index = self.buffer.len;

            return std.fmt.parseInt(u32, element, 10) catch 0;
        }

        return null;
    }
};

fn processFaceIndices(indices: []const u8) [3]u32 {
    var parser: FaceIndexParser = .{
        .buffer = indices,
    };

    var result: [3]u32 = .{ 0, 0, 0 };
    result[0] = parser.next() orelse 0;
    result[1] = parser.next() orelse 0;
    result[2] = parser.next() orelse 0;
    return result;
}

test "parse obj data" {
    const data =
        \\v 1.000000 1.000000 1.000000
        \\v 2.000000 2.000000 2.000000
        \\v 3.000000 3.000000 3.000000 9.000000
        \\vt 0.000000 0.000000
        \\vt 0.000000 1.000000 0.500000
        \\vt 0.500000 1.000000
        \\vn 0.000000 0.800000 0.750000
        \\vn 0.123000 0.000000 1.000000
        \\vn 2.000000 0.200000 0.333333
        \\f 3/1/1 1/2/3 2/3/2
        \\f 1//1 2/2 3
    ;

    const allocator = std.testing.allocator;
    var model = try loadData(allocator, data);
    defer model.deinit(allocator);

    const v1 = model.getVertexFace(0, 0) orelse unreachable;
    try std.testing.expect(v1.eql(.init(3.0, 3.0, 3.0, 9.0)));

    const v2 = model.getVertexFace(0, 1) orelse unreachable;
    try std.testing.expect(v2.eql(.init(1.0, 1.0, 1.0, 1.0)));

    const v3 = model.getVertexFace(0, 2) orelse unreachable;
    try std.testing.expect(v3.eql(.init(2.0, 2.0, 2.0, 1.0)));

    const v4 = model.getVertexFace(20, 0);
    try std.testing.expectEqual(null, v4);

    const tc1 = model.getTextureCoordFace(0, 0) orelse unreachable;
    try std.testing.expect(tc1.eql(.init(0.0, 0.0, 0.0)));

    const tc2 = model.getTextureCoordFace(0, 1) orelse unreachable;
    try std.testing.expect(tc2.eql(.init(0.0, 1.0, 0.5)));

    const tc3 = model.getTextureCoordFace(0, 2) orelse unreachable;
    try std.testing.expect(tc3.eql(.init(0.5, 1.0, 0.0)));

    const n1 = model.getNormalFace(0, 0) orelse unreachable;
    try std.testing.expect(n1.eql(.init(0.0, 0.8, 0.75)));

    const n2 = model.getNormalFace(0, 1) orelse unreachable;
    try std.testing.expect(n2.eql(.init(2.0, 0.2, 0.333333)));

    const n3 = model.getNormalFace(0, 2) orelse unreachable;
    try std.testing.expect(n3.eql(.init(0.123, 0.0, 1.0)));

    const invalid_face = model.getFace(2);
    try std.testing.expectEqual(null, invalid_face);

    const no_tex_coord = model.getTextureCoordFace(1, 0);
    try std.testing.expectEqual(null, no_tex_coord);
}
