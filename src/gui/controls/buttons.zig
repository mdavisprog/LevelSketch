const clay = @import("clay");
const controls = @import("root.zig");
const State = @import("../State.zig");

const Text = controls.Text;

pub fn label(state: *State, id: clay.ElementId, text: Text) void {
    clay.openElement();

    const button_color: clay.Color = blk: {
        if (state.pressed.eql(id)) {
            break :blk state.theme.colors.pressed;
        } else if (clay.hovered()) {
            break :blk state.theme.colors.hovered;
        } else {
            break: blk state.theme.colors.control;
        }
    };

    clay.configureOpenElement(.{
        .id = id,
        .layout = .{
        .sizing = .{
            .width = .percent(1.0),
        },
        .child_alignment = .{
            .x = .center,
            .y = .center,
        },
        .padding = .axes(4.0, 4.0),
    },
    .background_color = button_color,
    });
    {
        const config = clay.storeTextElementConfig(.{
            .font_id = text.font.id,
            .text_alignment = .center,
            .font_size = 18,
        });
        clay.openTextElement(text.contents, config);
    }
    clay.closeElement();
}
