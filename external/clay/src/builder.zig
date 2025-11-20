const clay = @import("root.zig");
const std = @import("std");

const Dimensions = clay.Dimensions;

var _element_stack_count: usize = 0;

pub fn begin() void {
    _element_stack_count = 0;
    clay.beginLayout();
}

pub fn end() clay.RenderCommandArray {
    if (_element_stack_count != 0) {
        std.debug.panic("{} elements still open.", .{_element_stack_count});
    }

    return clay.endLayout();
}

pub fn beginElement(comptime id: []const u8, element: clay.ElementDeclaration) void {
    var element_with_id = element;
    element_with_id.id = clay.id(id);

    clay.openElement();
    clay.configureOpenElement(element);

    _element_stack_count += 1;
}

pub fn beginTextElement(comptime text: []const u8, element: clay.TextElementConfig) void {
    const config = clay.storeTextElementConfig(element);
    clay.openTextElement(text, config);
    _element_stack_count += 1;
}

pub fn endElement() void {
    if (_element_stack_count == 0) {
        std.debug.panic("Trying to close element that isn't open.", .{});
    }

    // Clay does not need the final closeElement tag as EndLayout will close it for us.
    if (_element_stack_count > 1) {
        clay.closeElement();
    }

    _element_stack_count -= 1;
}
