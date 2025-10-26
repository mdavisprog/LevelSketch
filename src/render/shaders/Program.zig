const builtin = @import("builtin");
const std = @import("std");
const zbgfx = @import("zbgfx");

const Self = @This();
const SHADER_PATH = "assets/shaders";

pub const Paths = struct {
    varying_file_name: []const u8,
    fragment_file_name: []const u8,
    vertex_file_name: []const u8,
};

handle: zbgfx.bgfx.ProgramHandle = .{ .idx = 0 },

pub fn build(self: *Self, allocator: std.mem.Allocator, paths: Paths) !zbgfx.bgfx.ProgramHandle {
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

    self.handle = zbgfx.bgfx.createProgram(vertex_shader, fragment_shader, true);
    return self.handle;
}

pub fn clean(self: *Self) void {
    zbgfx.bgfx.destroyProgram(self.handle);
}

fn getContents(filename: []const u8, allocator: std.mem.Allocator) ![]u8 {
    const exe_dir = try std.fs.selfExeDirPathAlloc(allocator);
    defer allocator.free(exe_dir);

    const path = try std.fs.path.join(allocator, &.{ exe_dir, SHADER_PATH, filename });
    defer allocator.free(path);

    const file = try std.fs.openFileAbsolute(path, .{});
    defer file.close();

    const file_size = try file.getEndPos();
    var buffer: [1024]u8 = undefined;
    var reader = file.reader(&buffer);
    return try reader.interface.readAlloc(allocator, file_size);
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
