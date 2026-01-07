const clay = @import("clay");
const controls = @import("root.zig");
const render = @import("render");
const State = @import("../State.zig");

const Texture = render.Texture;

pub fn image(
    state: *State,
    id: clay.ElementId,
    texture: Texture,
    color: clay.Color,
) void {
    imageSized(
        state,
        id,
        texture,
        color,
        .init(
            @floatFromInt(texture.width),
            @floatFromInt(texture.height),
        ),
    );
}

pub fn imageSized(
    _: *State,
    id: clay.ElementId,
    texture: Texture,
    color: clay.Color,
    size: clay.Vector2,
) void {
    clay.openElement();
    clay.configureOpenElement(.{
        .id = id,
        .layout = .{
            .sizing = .fixed(size.x, size.y),
        },
        .image = .{
            .image_data = @ptrFromInt(texture.id),
        },
        .background_color = color,
    });
    clay.closeElement();
}
