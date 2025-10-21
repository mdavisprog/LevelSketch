const core = @import("core");
const std = @import("std");
const zbgfx = @import("zbgfx");

const commandline = core.commandline;

const CCallbackInterfaceT = zbgfx.callbacks.CCallbackInterfaceT;
const CCallbackVtblT = zbgfx.callbacks.CCallbackVtblT;
const formatTrace = zbgfx.callbacks.formatTrace;
const VaList = zbgfx.callbacks.VaList;

pub const BGFXCallbacks = struct {
    const Self = @This();

    pub fn fatal(
        _this: *CCallbackInterfaceT,
        _filePath: [*:0]const u8,
        _line: u16,
        _code: zbgfx.bgfx.Fatal,
        c_str: [*:0]const u8,
    ) callconv(.c) void {
        _ = _this;
        const cstr = std.mem.span(c_str);
        std.log.err("FATAL in {s}:{d}: {s} => {s}", .{ _filePath, _line, @tagName(_code), cstr });
    }
    pub fn trace_vargs(
        _this: *CCallbackInterfaceT,
        _filePath: [*:0]const u8,
        _line: u16,
        _format: [*:0]const u8,
        va_list: VaList,
    ) callconv(.c) void {
        if (!commandline.hasArg("--bgfx-log")) {
            return;
        }

        _ = _this;
        _ = _filePath;
        _ = _line;

        const bgfx_string = "BGFX ";

        var buff: [1024]u8 = undefined;
        const len = formatTrace(&buff, buff.len, _format, va_list);
        const msg = buff[bgfx_string.len .. @as(usize, @intCast(len)) - 1];

        std.log.debug("{s}", .{msg});
    }
    pub fn profiler_begin(
        _this: *CCallbackInterfaceT,
        _name: [*:0]const u8,
        _abgr: u32,
        _filePath: [*:0]const u8,
        _line: u16,
    ) callconv(.c) void {
        _ = _this;
        _ = _name;
        _ = _abgr;
        _ = _filePath;
        _ = _line;
    }
    pub fn profiler_begin_literal(
        _this: *CCallbackInterfaceT,
        _name: [*:0]const u8,
        _abgr: u32,
        _filePath: [*:0]const u8,
        _line: u16,
    ) callconv(.c) void {
        _ = _this;
        _ = _name;
        _ = _abgr;
        _ = _filePath;
        _ = _line;
    }
    pub fn profiler_end(_this: *CCallbackInterfaceT) callconv(.c) void {
        _ = _this;
    }
    pub fn cache_read_size(_this: *CCallbackInterfaceT, _id: u64) callconv(.c) u32 {
        _ = _this;
        _ = _id;
        return 0;
    }
    pub fn cache_read(
        _this: *CCallbackInterfaceT,
        _id: u64,
        _data: [*c]u8,
        _size: u32,
    ) callconv(.c) bool {
        _ = _this;
        _ = _id;
        _ = _data;
        _ = _size;
        return false;
    }
    pub fn cache_write(
        _this: *CCallbackInterfaceT,
        _id: u64,
        _data: [*c]u8,
        _size: u32,
    ) callconv(.c) void {
        _ = _this;
        _ = _id;
        _ = _data;
        _ = _size;
    }
    pub fn screen_shot(_this: *CCallbackInterfaceT, _filePath: [*:0]const u8, _width: u32, _height: u32, _pitch: u32, _data: [*c]u8, _size: u32, _yflip: bool) callconv(.c) void {
        _ = _this;
        _ = _filePath;
        _ = _width;
        _ = _height;
        _ = _pitch;
        _ = _data;
        _ = _size;
        _ = _yflip;
    }
    pub fn capture_begin(
        _this: *CCallbackInterfaceT,
        _width: u32,
        _height: u32,
        _pitch: u32,
        _format: zbgfx.bgfx.TextureFormat,
        _yflip: bool,
    ) callconv(.c) void {
        _ = _this;
        _ = _width;
        _ = _height;
        _ = _pitch;
        _ = _format;
        _ = _yflip;
        std.log.warn("{s}", .{"Using capture without callback (a.k.a. pointless)."});
    }
    pub fn capture_end(_this: *CCallbackInterfaceT) callconv(.c) void {
        _ = _this;
    }
    pub fn capture_frame(
        _this: *CCallbackInterfaceT,
        _data: [*c]u8,
        _size: u32,
    ) callconv(.c) void {
        _ = _this;
        _ = _data;
        _ = _size;
    }

    pub fn toVtbl() CCallbackVtblT {
        return CCallbackVtblT{
            .fatal = Self.fatal,
            .trace_vargs = Self.trace_vargs,
            .profiler_begin = Self.profiler_begin,
            .profiler_begin_literal = Self.profiler_begin_literal,
            .profiler_end = Self.profiler_end,
            .cache_read_size = Self.cache_read_size,
            .cache_read = Self.cache_read,
            .cache_write = Self.cache_write,
            .screen_shot = Self.screen_shot,
            .capture_begin = Self.capture_begin,
            .capture_end = Self.capture_end,
            .capture_frame = Self.capture_frame,
        };
    }
};
