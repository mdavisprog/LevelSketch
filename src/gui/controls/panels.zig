const clay = @import("clay");
const core = @import("core");
const State = @import("../State.zig");
const std = @import("std");

const Rectf = core.math.Rectf;
const Vec2f = core.math.Vec2f;

pub const Panel = struct {
    bounds: Rectf = .zero,
};

const default_bounds: Rectf = .init(20.0, 20.0, 200.0, 200.0);

pub fn begin(state: *State, id: clay.ElementId, title: []const u8) void {
    const data = state.getOrSetData(id, .{
        .panel = .{
            .bounds = default_bounds,
        },
    });

    const bounds = data.panel.bounds;

    clay.openElement();
    clay.configureOpenElement(.{
        .layout = .{
            .sizing = .fixed(bounds.width(), bounds.height()),
            .layout_direction = .top_to_bottom,
            .child_gap = 4,
            .padding = .axes(4, 2),
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
            .padding = .axes(2, 4),
        },
    });
    {
        const config = clay.storeTextElementConfig(.{
            .font_id = @intCast(state.theme.font.handle.id),
            .font_size = state.theme.font_sizes.header,
            .text_alignment = .center,
        });
        clay.openTextElement(title, config);
    }
    clay.closeElement();
}

fn onDragTitle(delta: Vec2f, context: State.Context) void {
    const data = context.state.getDataMut(context.element) orelse return;
    data.panel.bounds.move(delta);
}
