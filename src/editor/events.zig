const core = @import("core");
const editor = @import("root.zig");
const world = @import("world");

const Camera = editor.components.Camera;
const Query = world.Query;
const SystemParam = world.SystemParam;
const Transform = world.components.core.Transform;
const Vec = core.math.Vec;

pub const ResetCamera = struct {};

pub fn onResetCamera(
    _: ResetCamera,
    cameras: Query(&.{ Camera, Transform }),
    param: SystemParam,
) !void {
    var entities = cameras.getEntities();
    while (entities.next()) |entity| {
        const transform = param.getComponent(Transform, entity.*) orelse continue;
        transform.translation = Camera.default_pos;
        transform.rotation = Vec.forward.toRotation();
    }
}
