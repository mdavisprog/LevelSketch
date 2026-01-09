const clay = @import("clay");
const controls = @import("root.zig");
const render = @import("render");
const State = @import("../State.zig");
const std = @import("std");

const Texture = render.Texture;

pub const Checkbox = struct {
    on_click: ?State.OnPointerEvent = null,
    checked: bool = false,
};

pub fn check(
    state: *State,
    id: clay.ElementId,
    text: []const u8,
    on_click: ?State.OnPointerEvent,
) void {
    state.updateId(id, .{ .on_click = onClick });
    const data = state.getOrSetData(id, .{
        .checkbox = .{
            .checked = false,
            .on_click = on_click,
        },
    });

    // Main container.
    clay.openElement();
    clay.configureOpenElement(.{
        .layout = .{
            .layout_direction = .left_to_right,
            .child_alignment = .{
                .y = .center,
            },
            .child_gap = 4,
        },
    });
    {
        // Background box.
        clay.openElement();

        const color: clay.Color = blk: {
            if (state.pressed.eql(id)) {
                break :blk state.theme.colors.pressed;
            } else if (clay.hovered()) {
                break :blk state.theme.colors.hovered;
            } else {
                break :blk state.theme.colors.control;
            }
        };

        clay.configureOpenElement(.{
            .id = id,
            .layout = .{
                .padding = .splat(4),
                .child_alignment = .{
                    .x = .center,
                    .y = .center,
                },
            },
            .background_color = color,
        });
        {
            // Check mark
            const icon = state.theme.getIcon("check");
            if (data.checkbox.checked) {
                controls.images.image(state, icon, .white);
            } else {
                controls.dummy(@floatFromInt(icon.width), @floatFromInt(icon.height));
            }
        }

        clay.closeElement();

        // Label
        const config = clay.storeTextElementConfig(.{
            .font_id = state.theme.font.id,
            .font_size = state.theme.font_sizes.normal,
        });
        clay.openTextElement(text, config);
    }
    clay.closeElement();
}

fn onClick(context: State.Context) bool {
    const data = context.state.getDataMut(context.element) orelse return false;

    data.checkbox.checked = !data.checkbox.checked;
    if (data.checkbox.on_click) |on_click| {
        return on_click(context);
    }

    return true;
}
