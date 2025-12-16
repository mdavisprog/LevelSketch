const clay = @import("clay");
const core = @import("core");
const State = @import("../State.zig");
const std = @import("std");

const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;

const default_bounds: Rectf = .init(20.0, 20.0, 200.0, 200.0);

pub fn begin(state: *State, id: clay.ElementId, title: []const u8) void {
    const bounds = state.registerData(id, default_bounds);

    clay.openElement();
    clay.configureOpenElement(.{
        .layout = .{
            .sizing = .fixed(bounds.width(), bounds.height()),
            .layout_direction = .top_to_bottom,
            .child_gap = 4.0,
        },
        .background_color = state.theme.colors.background,
        .floating = .{
            .offset = .init(bounds.min.x, bounds.min.y),
            .attach_to = .parent,
        },
    });
    {
        // Title bar
        titleBar(id, title, state);
    }
}

pub fn end() void {
    clay.closeElement();
}

fn titleBar(id: clay.ElementId, title: []const u8, state: *State) void {
    state.registerId(id, .{
        .on_drag = onDragTitle,
    });

    clay.openElement();
    clay.configureOpenElement(.{
        .id = id,
        .layout = .{
            .child_alignment = .{
                .x = .center,
                .y = .center,
            },
            .sizing = .{
                .width = .percent(1.0),
            },
            .padding = .axes(2.0, 6.0),
        },
    });
    {
        const config = clay.storeTextElementConfig(.{
            .font_id = state.font.id,
            .text_alignment = .center,
        });
        clay.openTextElement(title, config);
    }
    clay.closeElement();
}

fn onDragTitle(delta: Vec2f, context: State.Context) void {
    const data = context.state.getDataMut(context.element) orelse return;
    data.rect.move(delta);
}
