const std = @import("std");

pub const Context = anyopaque;

pub const Arena = extern struct {
    nextAllocation: usize = 0,
    capacity: usize = 0,
    memory: [*c]const u8 = null,
};

pub const Vector2 = extern struct {
    x: f32 = 0.0,
    y: f32 = 0.0,
};

pub const Dimensions = extern struct {
    width: f32 = 0.0,
    height: f32 = 0.0,

    pub fn init(width: f32, height: f32) Dimensions {
        return .{
            .width = width,
            .height = height,
        };
    }
};

pub const BoundingBox = extern struct {
    x: f32 = 0.0,
    y: f32 = 0.0,
    width: f32 = 0.0,
    height: f32 = 0.0,
};

pub const Color = extern struct {
    r: f32 = 0.0,
    g: f32 = 0.0,
    b: f32 = 0.0,
    a: f32 = 0.0,

    pub fn init(r: f32, g: f32, b: f32, a: f32) Color {
        return .{
            .r = r,
            .g = g,
            .b = b,
            .a = a,
        };
    }

    pub fn initu8(r: u8, g: u8, b: u8, a: u8) Color {
        return .{
            .r = @floatFromInt(r),
            .g = @floatFromInt(g),
            .b = @floatFromInt(b),
            .a = @floatFromInt(a),
        };
    }
};

pub const CornerRadius = extern struct {
    top_left: f32 = 0.0,
    top_right: f32 = 0.0,
    bottom_left: f32 = 0.0,
    bottom_right: f32 = 0.0,
};

pub const String = extern struct {
    is_statically_allocated: bool = true,
    length: i32 = 0,
    chars: [*c]const u8 = null,

    pub fn init(comptime chars: []const u8) String {
        return .{
            .is_statically_allocated = true,
            .length = @intCast(chars.len),
            .chars = chars.ptr,
        };
    }

    pub fn str(self: String) []const u8 {
        const len: usize = @intCast(self.length);
        return self.chars[0..len];
    }
};

pub const StringSlice = extern struct {
    length: i32 = 0,
    chars: [*c]const u8 = null,
    base_chars: [*c]const u8 = null,
};

pub const ElementId = extern struct {
    id: u32 = 0,
    offset: u32 = 0,
    base_id: u32 = 0,
    string_id: String = .{},
};

pub const Sizing = extern struct {
    pub const Type = enum(u8) {
        fit,
        grow,
        percent,
        fixed,
    };

    pub const MinMax = extern struct {
        min: f32 = 0.0,
        max: f32 = 0.0,
    };

    pub const Axis = extern struct {
        pub const Size = extern union {
            min_max: MinMax,
            percent: f32,
        };

        size: Size = std.mem.zeroes(Size),
        type: Type = .fit,

        pub fn fit(min: f32, max: f32) Axis {
            return .{
                .size = .{
                    .min_max = .{ .min = min, .max = max },
                },
                .type = .fit,
            };
        }

        pub fn grow(min: f32, max: f32) Axis {
            return .{
                .size = .{
                    .min_max = .{ .min = min, .max = max },
                },
                .type = .grow,
            };
        }

        pub fn fixed(size: f32) Axis {
            return .{
                .size = .{
                    .min_max = .{ .min = size, .max = size },
                },
                .type = .fixed,
            };
        }

        pub fn percent(value: f32) Axis {
            return .{
                .size = .{
                    .percent = value,
                },
                .type = .percent,
            };
        }
    };

    width: Axis = .{},
    height: Axis = .{},

    pub fn fixed(width: f32, height: f32) Sizing {
        return .{
            .width = .fixed(width),
            .height = .fixed(height),
        };
    }
};

pub const Padding = extern struct {
    left: u16 = 0,
    right: u16 = 0,
    top: u16 = 0,
    bottom: u16 = 0,

    pub fn splat(value: u16) Padding {
        return .{
            .left = value,
            .right = value,
            .top = value,
            .bottom = value,
        };
    }
};

pub const LayoutDirection = enum(u8) {
    left_to_right,
    top_to_bottom,
};

pub const LayoutAlignmentX = enum(u8) {
    left,
    right,
    center,
};

pub const LayoutAlignmentY = enum(u8) {
    top,
    bottom,
    center,
};

pub const ChildAlignment = extern struct {
    x: LayoutAlignmentX = .left,
    y: LayoutAlignmentY = .top,
};

pub const LayoutConfig = extern struct {
    sizing: Sizing = .{},
    padding: Padding = .{},
    child_gap: u16 = 0,
    child_alignment: ChildAlignment = .{},
    layout_direction: LayoutDirection = .left_to_right,
};

pub const AspectRatioElementConfig = extern struct {
    aspect_ratio: f32 = 0.0,
};

pub const ImageElementConfig = extern struct {
    image_data: ?*anyopaque = null,
};

pub const FloatingAttachPoints = extern struct {
    pub const Type = enum(u8) {
        left_top,
        left_center,
        left_bottom,
        center_top,
        center_center,
        center_bottom,
        right_top,
        right_center,
        right_bottom,
    };

    element: Type = .left_top,
    parent: Type = .left_top,
};

pub const PointerCaptureMode = enum(u8) {
    capture,
    passthrough,
};

pub const FloatingAttachToElement = enum(u8) {
    none,
    parent,
    element_with_id,
    root,
};

pub const FloatingClipToElement = enum(u8) { none, attached_parent };

pub const FloatingElementConfig = extern struct {
    offset: Vector2 = .{},
    expand: Dimensions = .{},
    parent_id: u32 = 0,
    z_index: i16 = 0,
    attach_points: FloatingAttachPoints = .{},
    pointer_capture_mode: PointerCaptureMode = .capture,
    attach_to: FloatingAttachToElement = .none,
    clip_to: FloatingClipToElement = .none,
};

pub const CustomElementConfig = extern struct {
    custom_data: ?*anyopaque = null,
};

pub const ClipElementConfig = extern struct {
    horizontal: bool = false,
    vertical: bool = false,
    child_offset: Vector2 = .{},
};

pub const BorderWidth = extern struct {
    left: u16 = 0,
    right: u16 = 0,
    top: u16 = 0,
    bottom: u16 = 0,
    betweenChildren: u16 = 0,
};

// Controls settings related to element borders.
pub const BorderElementConfig = extern struct {
    color: Color = .{},
    width: BorderWidth = .{},
};

pub const ElementDeclaration = extern struct {
    id: ElementId = .{},
    layout: LayoutConfig = .{},
    background_color: Color = .{},
    corner_radius: CornerRadius = .{},
    aspect_ratio: AspectRatioElementConfig = .{},
    image: ImageElementConfig = .{},
    floating: FloatingElementConfig = .{},
    custom: CustomElementConfig = .{},
    clip: ClipElementConfig = .{},
    border: BorderElementConfig = .{},
    user_data: ?*anyopaque = null,
};

pub const RectangleRenderData = extern struct {
    background_color: Color = .{},
    corner_radius: CornerRadius = .{},
};

pub const TextRenderData = extern struct {
    string_contents: StringSlice = .{},
    text_color: Color = .{},
    font_id: u16 = 0,
    font_size: u16 = 0,
    letter_spacing: u16 = 0,
    line_height: u16 = 0,
};

pub const ImageRenderData = extern struct {
    background_color: Color = .{},
    corner_radius: CornerRadius = .{},
    image_data: ?*anyopaque = null,
};

pub const CustomRenderData = extern struct {
    background_color: Color = .{},
    corner_radius: CornerRadius = .{},
    custom_data: ?*anyopaque = null,
};

pub const BorderRenderData = extern struct {
    color: Color = .{},
    corner_radius: CornerRadius = .{},
    width: BorderWidth = .{},
};

pub const ClipRenderData = extern struct {
    horizontal: bool = false,
    vertical: bool = false,
};

pub const RenderData = extern union {
    rectangle: RectangleRenderData,
    text: TextRenderData,
    image: ImageRenderData,
    custom: CustomRenderData,
    border: BorderRenderData,
    clip: ClipRenderData,

    pub fn new() RenderData {
        return std.mem.zeroes(RenderData);
    }
};

pub const RenderCommand = extern struct {
    pub const Type = enum(u8) {
        none,
        rectangle,
        border,
        text,
        image,
        scissor_start,
        scissor_end,
        custom,
    };

    bounding_box: BoundingBox = .{},
    render_data: RenderData = .new(),
    user_data: ?*anyopaque = null,
    id: u32 = 0,
    z_index: i16 = 0,
    command_type: Type = .none,
};

pub const RenderCommandArray = extern struct {
    capacity: i32 = 0,
    length: i32 = 0,
    internal_array: [*c]RenderCommand = null,

    pub fn slice(self: RenderCommandArray) []const RenderCommand {
        const len: usize = @intCast(self.length);
        return self.internal_array[0..len];
    }

    pub fn get(self: *RenderCommandArray, index: usize) *RenderCommand {
        return @ptrCast(RenderCommandArray_Get(self, @intCast(index)));
    }
};

pub const ErrorData = extern struct {
    pub const Type = enum(u8) {
        text_measurement_function_not_provided,
        arena_capacity_exceeded,
        elements_capacity_exceeded,
        text_measurement_capacity_exceeded,
        duplicate_id,
        floating_container_parent_not_found,
        percentage_over_1,
        internal_error,
    };

    error_type: Type = .internal_error,
    error_text: String = .{},
    user_data: ?*anyopaque = null,
};

pub const ErrorHandler = extern struct {
    pub const Callback = *const fn (error_text: ErrorData) callconv(.c) void;

    error_handler_function: ?Callback = null,
    user_data: ?*anyopaque = null,
};

pub fn minMemorySize() u32 {
    return Clay_MinMemorySize();
}

pub fn createArenaWithCapacityAndMemory(capacity: usize, memory: ?*anyopaque) Arena {
    return Clay_CreateArenaWithCapacityAndMemory(capacity, memory);
}

pub fn initialize(arena: Arena, layout_dimensions: Dimensions, error_handler: ErrorHandler) ?*Context {
    return Clay_Initialize(arena, layout_dimensions, error_handler);
}

pub fn setLayoutDimensions(dimensions: Dimensions) void {
    Clay_SetLayoutDimensions(dimensions);
}

pub fn beginLayout() void {
    Clay_BeginLayout();
}

pub fn endLayout() RenderCommandArray {
    return Clay_EndLayout();
}

pub fn openElement() void {
    Clay__OpenElement();
}

pub fn closeElement() void {
    Clay__CloseElement();
}

pub fn configureOpenElement(config: ElementDeclaration) void {
    Clay__ConfigureOpenElement(config);
}

pub fn RenderCommandArray_Get(array: [*c]RenderCommandArray, index: i32) [*c]RenderCommand {
    return Clay_RenderCommandArray_Get(array, index);
}

pub fn id(comptime label: []const u8) ElementId {
    const str: String = .{
        .is_statically_allocated = true,
        .length = @intCast(label.len),
        .chars = label.ptr,
    };
    return hashString(str, 0, 0);
}

fn hashString(comptime key: String, comptime offset: u32, comptime seed: u32) ElementId {
    var hash: u32 = 0;
    var base: u32 = seed;

    for (0..key.length) |i| {
        base +%= key.chars[i];
        base +%= (base << 10);
        base ^= (base >> 6);
    }
    hash = base;
    hash +%= offset;
    hash +%= (hash << 10);
    hash ^= (hash >> 6);

    hash +%= (hash << 3);
    base +%= (base << 3);
    hash ^= (hash >> 11);
    base ^= (base >> 11);
    hash +%= (hash << 15);
    base +%= (base << 15);

    return .{
        .id = hash + 1,
        .offset = offset,
        .base_id = base + 1,
        .string_id = key,
    };
}

extern fn Clay_MinMemorySize() u32;
extern fn Clay_CreateArenaWithCapacityAndMemory(capacity: usize, memory: ?*anyopaque) Arena;
extern fn Clay_Initialize(arena: Arena, layout_dimension: Dimensions, error_handler: ErrorHandler) ?*Context;
extern fn Clay_SetLayoutDimensions(dimensions: Dimensions) void;
extern fn Clay_BeginLayout() void;
extern fn Clay_EndLayout() RenderCommandArray;
extern fn Clay_RenderCommandArray_Get(array: [*c]RenderCommandArray, index: i32) [*c]RenderCommand;
extern fn Clay__OpenElement() void;
extern fn Clay__CloseElement() void;
extern fn Clay__ConfigureOpenElement(config: ElementDeclaration) void;
