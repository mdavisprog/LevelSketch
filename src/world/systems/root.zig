const core = @import("core");
const std = @import("std");
const world = @import("../root.zig");

const components = world.components;
const Entity = world.Entity;
const Mat = core.math.Mat;
const Query = world.Query;
const SystemParam = world.SystemParam;

pub fn resolveTransforms(
    children: Query(&.{components.core.Transform, components.core.Child}),
    param: SystemParam,
) !void {
    var entities = children.getEntities();
    while (entities.next()) |entity| {
        const child = param.getComponent(components.core.Child, entity.*) orelse continue;

        const resolved_transform = try param.world.getOrInsertComponent(
            components.core.ResolvedTransform,
            entity.*,
        ) orelse {
            std.log.warn("Failed to insert ResolvedTransform to entity {}.", .{entity.id});
            continue;
        };

        resolved_transform.transform = resolveTransform(entity.*, child.*, param);
    }
}

fn resolveTransform(
    entity: Entity,
    child: components.core.Child,
    param: SystemParam,
) Mat {
    const transform = param.getComponent(
        components.core.Transform,
        entity,
    ) orelse return .identity;

    const parent = param.getComponent(
        components.core.Transform,
        child.parent,
    ) orelse return transform.toMatrix();

    return transform.toMatrix().mul(parent.toMatrix());
}

fn getResolvedTransform(entity: Entity, param: SystemParam) !*components.core.ResolvedTransform {
    if (param.world.getComponent(components.core.ResolvedTransform, entity)) |result| {
        return result;
    } else {
        try param.world.insertComponent(components.core.ResolvedTransform, entity, .{});
        return param.getComponent(components.core.ResolvedTransform, entity).?;
    }
}
