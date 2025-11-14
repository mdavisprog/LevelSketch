const builtin = @import("builtin");
const io = @import("io");
const std = @import("std");
const shaders = @import("root.zig");
const zbgfx = @import("zbgfx");

const Uniform = shaders.Uniform;

const Self = @This();

pub const UniformMap = std.StringHashMap(Uniform);

pub const Paths = struct {
    varying_file_name: []const u8,
    fragment_file_name: []const u8,
    vertex_file_name: []const u8,
};

pub const Handle = struct {
    data: zbgfx.bgfx.ProgramHandle = .{ .idx = 0 },
};

pub const Error = error{
    UniformNotFound,
};

const SHADER_PATH = "assets/shaders";

handle: Handle = .{},
uniforms: UniformMap,

pub fn init(gpa: std.mem.Allocator) !*Self {
    const result = try gpa.create(Self);
    result.*.uniforms = UniformMap.init(gpa);
    return result;
}

pub fn deinit(self: *Self) void {
    var keys = self.uniforms.keyIterator();
    while (keys.next()) |key| {
        self.uniforms.allocator.free(key.*);
    }

    self.uniforms.deinit();
    zbgfx.bgfx.destroyProgram(self.handle.data);
}

pub fn build(self: *Self, paths: Paths) !void {
    const allocator = self.uniforms.allocator;

    const shader_exe = try getShaderExePath(allocator);
    defer allocator.free(shader_exe);

    const defs = try getContents(paths.varying_file_name, allocator);
    defer allocator.free(defs);

    const common_fragment = try getContents(paths.fragment_file_name, allocator);
    defer allocator.free(common_fragment);

    const common_vertex = try getContents(paths.vertex_file_name, allocator);
    defer allocator.free(common_vertex);

    const exe_dir = try std.fs.selfExeDirPathAlloc(allocator);
    defer allocator.free(exe_dir);

    const path = try std.fs.path.join(allocator, &.{ exe_dir, SHADER_PATH, "include" });
    defer allocator.free(path);

    const renderer_type = zbgfx.bgfx.getRendererType();
    var fragment_options = zbgfx.shaderc.createDefaultOptionsForRenderer(renderer_type);
    fragment_options.shaderType = .fragment;
    fragment_options.includeDirs = &.{path};

    const fragment_compiled = try zbgfx.shaderc.compileShader(
        allocator,
        shader_exe,
        defs,
        common_fragment,
        fragment_options,
    );
    defer allocator.free(fragment_compiled);

    var vertex_options = zbgfx.shaderc.createDefaultOptionsForRenderer(renderer_type);
    vertex_options.shaderType = .vertex;
    vertex_options.includeDirs = &.{path};

    const vertex_compiled = try zbgfx.shaderc.compileShader(
        allocator,
        shader_exe,
        defs,
        common_vertex,
        vertex_options,
    );
    defer allocator.free(vertex_compiled);

    const fragment_shader = zbgfx.bgfx.createShader(zbgfx.bgfx.copy(
        fragment_compiled.ptr,
        @intCast(fragment_compiled.len),
    ));

    const vertex_shader = zbgfx.bgfx.createShader(zbgfx.bgfx.copy(
        vertex_compiled.ptr,
        @intCast(vertex_compiled.len),
    ));

    self.handle.data = zbgfx.bgfx.createProgram(vertex_shader, fragment_shader, false);

    try getUniforms(&self.uniforms, fragment_shader);
    try getUniforms(&self.uniforms, vertex_shader);

    zbgfx.bgfx.destroyShader(fragment_shader);
    zbgfx.bgfx.destroyShader(vertex_shader);
}

pub fn getUniform(self: Self, name: []const u8) !Uniform {
    if (self.uniforms.get(name)) |uniform| {
        return uniform;
    }

    return Error.UniformNotFound;
}

fn getContents(filename: []const u8, allocator: std.mem.Allocator) ![]u8 {
    const exe_dir = try std.fs.selfExeDirPathAlloc(allocator);
    defer allocator.free(exe_dir);

    const path = try io.exeRelativePath(allocator, &.{ SHADER_PATH, filename });
    defer allocator.free(path);

    return try io.getContents(allocator, path);
}

fn getShaderExePath(allocator: std.mem.Allocator) ![]u8 {
    const exe_dir = try std.fs.selfExeDirPathAlloc(allocator);
    defer allocator.free(exe_dir);

    const path = try std.fs.path.join(allocator, &.{ exe_dir, "shaderc" });

    if (builtin.os.tag == .windows) {
        defer allocator.free(path);
        return try std.fmt.allocPrint(allocator, "{s}.exe", .{path});
    }

    return path;
}

fn getUniforms(map: *UniformMap, shader: zbgfx.bgfx.ShaderHandle) !void {
    const count = zbgfx.bgfx.getShaderUniforms(shader, null, 32);

    if (count == 0) {
        return;
    }

    var uniforms = try map.allocator.alloc(zbgfx.bgfx.UniformHandle, count);
    defer map.allocator.free(uniforms);

    _ = zbgfx.bgfx.getShaderUniforms(shader, @ptrCast(&uniforms[0]), count);

    for (0..count) |i| {
        const uniform = uniforms[i];

        var info = std.mem.zeroes(zbgfx.bgfx.UniformInfo);
        zbgfx.bgfx.getUniformInfo(uniform, @ptrCast(&info));

        // Find the sentinel terminator.
        var len: usize = 0;
        for (0..info.name.len) |j| {
            if (info.name[j] == 0) {
                break;
            }

            len += 1;
        }

        const name: []const u8 = info.name[0..len];

        if (map.contains(name)) {
            try map.put(name, .{ .handle = uniform });
        } else {
            const key = try map.allocator.dupe(u8, name);
            try map.put(key, .{ .handle = uniform });
        }
    }
}
