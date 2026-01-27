const core = @import("core");
const editor = @import("root.zig");
const platform = @import("platform");
const render = @import("render");
const std = @import("std");
const world = @import("world");

const Mat = core.math.Mat;
const Renderer = render.Renderer;
const SystemParam = world.Systems.SystemParam;
const Vec = core.math.Vec;

pub fn updateCamera(param: SystemParam) !void {
    const frame = param.world.getResource(world.resources.core.Frame) orelse return;
    const _platform = param.world.getResource(platform.resources.Platform) orelse return;
    const keyboard = param.world.getResource(platform.input.resources.Keyboard) orelse return;
    const mouse = param.world.getResource(platform.input.resources.Mouse) orelse return;
    const _render = param.world.getResource(render.ecs.resources.Render) orelse return;

    var entities = param.entities.iterator();
    var submitted = false;
    while (entities.next()) |entity| {
        const transform = param.world.getComponent(world.components.core.Transform, entity.*) orelse continue;
        const camera = param.world.getComponent(editor.components.Camera, entity.*) orelse continue;

        const rotation = transform.rotation.toVec();
        const direction: Vec = if (keyboard.state.isPressed(.w))
            rotation
        else if (keyboard.state.isPressed(.s))
            rotation.mul(-1.0)
        else if (keyboard.state.isPressed(.d))
            rotation.cross(Vec.up).mul(-1.0)
        else if (keyboard.state.isPressed(.a))
            rotation.cross(Vec.up)
        else if (keyboard.state.isPressed(.q))
            Vec.up
        else if (keyboard.state.isPressed(.e))
            Vec.up.mul(-1.0)
        else
            .zero;

        camera.velocity.addMut(direction.mul(camera.move_speed));
        camera.velocity.clampMut(camera.max_speed);
        transform.translation.addMut(camera.velocity.mul(frame.times.delta));

        const friction: f32 = 0.9;
        camera.velocity.mulMut(friction);

        if (mouse.state.pressed(.right)) {
            const delta = mouse.state.delta();
            if (!delta.isZero()) {
                if (!camera.is_rotating) {
                    try _platform.primary_window.toggleCursor(false);
                    camera.is_rotating = true;
                }

                transform.rotation.pitch += -delta.y * camera.rotation_speed;
                transform.rotation.yaw += -delta.x * camera.rotation_speed;
            }
        } else if (mouse.state.released(.right)) {
            if (camera.is_rotating) {
                try _platform.primary_window.toggleCursor(true);
                camera.is_rotating = false;
            }
        }

        // TODO: Multiple viewports support.
        if (!submitted) {
            submitted = true;

            const look_at: Mat = .initLookToLh(transform.translation, transform.rotation.toVec(), Vec.up);
            const framebuffer_size = _platform.primary_window.framebufferSize();
            _render.renderer.updateView(framebuffer_size);
            _render.renderer.view_world.submitPerspective(
                look_at,
                @intCast(framebuffer_size.x),
                @intCast(framebuffer_size.y),
            );

            if (_render.renderer.programs.getByName("phong")) |phong| {
                const u_view_pos = try phong.getUniform("u_view_pos");
                u_view_pos.setArray(&transform.translation.toArray());
            }
        }
    }
}

pub fn orbit(param: SystemParam) !void {
    const resource = param.world.getResource(editor.resources.Orbit) orelse unreachable;
    if (!resource.enabled) {
        return;
    }

    const frame = param.world.getResource(world.resources.core.Frame) orelse unreachable;
    const delta_time = frame.times.delta;

    var entities = param.entities.iterator();
    while (entities.next()) |entity| {
        const transform = param.world.getComponent(world.components.core.Transform, entity.*) orelse continue;
        const _orbit = param.world.getComponent(editor.components.Orbit, entity.*) orelse continue;

        _orbit.elapsed += delta_time;
        _orbit.yaw = @mod(_orbit.yaw + _orbit.speed * delta_time, 360.0);
        const sin = std.math.sin(_orbit.elapsed);

        transform.translation = Mat.initTranslation(.right)
            .rotateY(_orbit.yaw)
            .scale(.splat(2.0))
            .getTranslation();
        transform.translation.setY(sin);
    }
}
