use bevy::{
    ecs::query::QueryData,
    picking::hover::HoverMap,
    prelude::*,
};
use crate::{
    animation::{
        KeaAnimation,
        KeaAnimationClip,
    },
    style,
};
use super::{
    events::{
        PlayAnimation,
        UpdateScrollbars,
    },
    KeaScrollable,
    resources::KeaScrollableResource,
    scrollbar::{
        Scrollbar,
        ScrollbarHoverState,
        ScrollbarsContainer,
        ScrollbarType,
    },
};

#[derive(QueryData)]
pub(super) struct ScrollableQuery {
    entity: Entity,
    component: &'static KeaScrollable,
    computed_node: &'static ComputedNode,
    transform: &'static GlobalTransform,
}

pub(super) fn build(app: &mut App) {
    app
        .add_systems(Update, on_changed_computed_node)
        .add_observer(on_released)
        .add_observer(update_scrollbars)
        .add_observer(on_play_animation);
}

pub(super) mod scrollable {
    use super::*;

    pub fn on_move(
        trigger: Trigger<Pointer<Move>>,
        computed_nodes: Query<(&ComputedNode, &GlobalTransform)>,
        parents: Query<&Children>,
        containers: Query<&ScrollbarsContainer>,
        resource: Res<KeaScrollableResource>,
        mut scrollbars: Query<(&mut Scrollbar, &Node)>,
        mut commands: Commands,
    ) {
        if resource.active_scrollbar.is_some() {
            return;
        }

        let Ok((computed_node, transform)) = computed_nodes.get(trigger.target()) else {
            return;
        };

        let bounds = Rect::from_center_size(transform.translation().truncate(), computed_node.size);
        let position = trigger.pointer_location.position;

        // Find the ScrollbarsContainer entity and loop through their children for the scrollbars.
        for child in parents.iter_descendants(trigger.target()) {
            if !containers.contains(child) {
                continue;
            }

            // Loop through each scrollbar.
            for entity in parents.iter_descendants(child) {
                let Ok((mut scrollbar, node)) = scrollbars.get_mut(entity) else {
                    continue;
                };

                let visible = {
                    let val = match scrollbar.bar_type() {
                        ScrollbarType::Horizontal => node.width,
                        ScrollbarType::Vertical => node.height,
                    };

                    if let Val::Px(value) = val {
                        value > 0.0
                    } else {
                        false
                    }
                };

                if !visible {
                    continue;
                }

                let hover_bounds = scrollbar.hover_bounds(bounds);
                if hover_bounds.contains(position) {
                    if scrollbar.hover_state == ScrollbarHoverState::Collapsed {
                        commands.trigger_targets(PlayAnimation::Expand, entity);
                        scrollbar.hover_state = ScrollbarHoverState::Expanded;
                    }
                } else {
                    if scrollbar.hover_state == ScrollbarHoverState::Expanded {
                        commands.trigger_targets(PlayAnimation::Collapse, entity);
                        scrollbar.hover_state = ScrollbarHoverState::Collapsed;
                    }
                }
            }

            break;
        }
    }
}

pub(super) mod scrollbar {
    use super::*;

    pub fn on_over(
        trigger: Trigger<Pointer<Over>>,
        resource: Res<KeaScrollableResource>,
        mut commands: Commands,
    ) {
        if resource.active_scrollbar.is_some() {
            return;
        }

        commands
            .entity(trigger.target())
            .insert(BackgroundColor(style::colors::HIGHLIGHT));
    }

    pub fn on_out(
        trigger: Trigger<Pointer<Out>>,
        resource: Res<KeaScrollableResource>,
        mut commands: Commands,
    ) {
        if resource.active_scrollbar.is_some() {
            return;
        }

        commands
            .entity(trigger.target())
            .insert(BackgroundColor(style::colors::BUTTON_BACKGROUND));
    }

    pub fn on_drag(
        trigger: Trigger<Pointer<Drag>>,
        children: Query<&ChildOf>,
        scrollbars: Query<&Scrollbar>,
        scrollables: Query<&KeaScrollable>,
        scroll_positions: Query<&ScrollPosition>,
        mut commands: Commands,
    ) {
        let Ok(scrollbar) = scrollbars.get(trigger.target()) else {
            return;
        };

        for parent in children.iter_ancestors(trigger.target()) {
            if !scrollables.contains(parent) {
                continue;
            }

            let Ok(scroll_position) = scroll_positions.get(parent) else {
                return;
            };

            let delta = trigger.delta;
            let offset = match scrollbar.bar_type() {
                ScrollbarType::Horizontal => Vec2::new(
                    scroll_position.offset_x + delta.x,
                    scroll_position.offset_y,
                ),
                ScrollbarType::Vertical => Vec2::new(
                    scroll_position.offset_x,
                    scroll_position.offset_y + delta.y,
                ),
            };

            commands
                .entity(parent)
                .insert(ScrollPosition {
                    offset_x: offset.x,
                    offset_y: offset.y,
                });
            commands.trigger_targets(UpdateScrollbars, parent);
        }
    }

    pub fn on_pressed(
        trigger: Trigger<Pointer<Pressed>>,
        mut resource: ResMut<KeaScrollableResource>,
    ) {
        resource.active_scrollbar = Some(trigger.target());
    }
}

fn on_released(
    trigger: Trigger<Pointer<Released>>,
    hover_map: Res<HoverMap>,
    scrollbars: Query<&Scrollbar>,
    mut resource: ResMut<KeaScrollableResource>,
    mut commands: Commands,
) {
    if let Some(active_scrollbar) = resource.active_scrollbar {
        let hovered = &hover_map[&trigger.pointer_id];

        if !hovered.contains_key(&active_scrollbar) {
            commands
                .entity(active_scrollbar)
                .insert(BackgroundColor(style::colors::BUTTON_BACKGROUND));

            for (entity, _) in hovered {
                if scrollbars.contains(*entity) {
                    commands
                        .entity(*entity)
                        .insert(BackgroundColor(style::colors::HIGHLIGHT));
                    break;
                }
            }
        }
    }

    resource.active_scrollbar = None;
}

fn on_changed_computed_node(
    changed: Query<Entity, Changed<ComputedNode>>,
    children: Query<&ChildOf>,
    scrollables: Query<&KeaScrollable>,
    mut commands: Commands,
) {
    for entity in changed {
        if scrollables.contains(entity) {
            commands.trigger_targets(UpdateScrollbars, entity);
        } else {
            for parent in children.iter_ancestors(entity) {
                if scrollables.contains(parent) {
                    commands.trigger_targets(UpdateScrollbars, parent);
                }
            }
        }
    }
}

fn update_scrollbars(
    trigger: Trigger<UpdateScrollbars>,
    scrollables: Query<ScrollableQuery>,
    parents: Query<&Children>,
    containers: Query<&ScrollbarsContainer>,
    animations: Query<&KeaAnimation>,
    scrollbars: Query<&Scrollbar>,
    mut nodes: Query<&mut Node>,
    mut scroll_positions: Query<&mut ScrollPosition>,
) {
    let Ok(scrollable) = scrollables.get(trigger.target()) else {
        return;
    };

    let Ok(mut scroll_position) = scroll_positions.get_mut(trigger.target()) else {
        return;
    };

    let Ok(parent) = parents.get(trigger.target()) else {
        return;
    };

    // Calculate the amount of scrollable space available.
    let size = scrollable.computed_node.size;
    let content_size = scrollable.computed_node.content_size;
    let overflow = (content_size - size).max(Vec2::ZERO);

    // Clamp the scroll position to the scrollable area.
    scroll_position.offset_x = scroll_position.offset_x.clamp(0.0, overflow.x);
    scroll_position.offset_y = scroll_position.offset_y.clamp(0.0, overflow.y);
    let scroll_offset = Vec2::new(scroll_position.offset_x, scroll_position.offset_y);

    // The scrollbar size is the difference between the scrollable area and the size of the
    // scrollable node. If there is no overflow, then no scrollbar will be visible.
    let mut scrollbar_sizes = (size - overflow).max(Vec2::new(10.0, 10.0));
    if overflow.x <= 0.0 { scrollbar_sizes.x = 0.0 }
    if overflow.y <= 0.0 { scrollbar_sizes.y = 0.0 }

    // Calculate the offset into the scrollable area and match it within the confines of the
    // node.
    let unit_offset = Vec2::new(
        if overflow.x > 0.0 { scroll_offset.x / overflow.x } else { 0.0 },
        if overflow.y > 0.0 { scroll_offset.y / overflow.y } else { 0.0 },
    );

    // Place the scrollbar within the node and take into account the size of the scrollbar.
    let shrink = if overflow.x > 0.0 { Vec2::new(0.0, 8.0) } else { Vec2::ZERO };
    let area = size - scrollbar_sizes - shrink;
    let position = (area * unit_offset + scroll_offset).max(Vec2::ZERO);

    for child in parent {
        if !containers.contains(*child) {
            continue;
        }

        let Ok(container) = parents.get(*child) else {
            continue;
        };

        for bar_entity in container {
            let Ok(scrollbar) = scrollbars.get(*bar_entity) else {
                continue;
            };

            let Ok(mut node) = nodes.get_mut(*bar_entity) else {
                continue;
            };

            let bar_size = match scrollbar.hover_state {
                ScrollbarHoverState::Collapsed => 2.0,
                ScrollbarHoverState::Expanded => 8.0,
            };

            match scrollbar.bar_type() {
                ScrollbarType::Horizontal => {
                    node.left = Val::Px(position.x);
                    node.width = Val::Px(scrollbar_sizes.x);

                    if !animations.contains(*bar_entity) {
                        node.top = Val::Px(scroll_position.offset_y + size.y - bar_size);
                        node.height = Val::Px(bar_size);
                    }
                },
                ScrollbarType::Vertical => {
                    node.top = Val::Px(position.y);
                    node.height = Val::Px(scrollbar_sizes.y);

                    if !animations.contains(*bar_entity) {
                        node.left = Val::Px(scroll_position.offset_x + size.x - bar_size);
                        node.width = Val::Px(bar_size);
                    }
                },
            }
        }

        break;
    }
}

fn on_play_animation(
    trigger: Trigger<PlayAnimation>,
    scrollbars: Query<&Scrollbar>,
    children: Query<&ChildOf>,
    computed_nodes: Query<(&ComputedNode, &ScrollPosition), With<KeaScrollable>>,
    mut commands: Commands,
) {
    let event = trigger.event();

    let Ok(scrollbar) = scrollbars.get(trigger.target()) else {
        return;
    };

    for parent in children.iter_ancestors(trigger.target()) {
        let Ok((computed_node, scroll_position)) = computed_nodes.get(parent) else {
            continue;
        };

        let bar_size = match event {
            PlayAnimation::Collapse => 2.0,
            PlayAnimation::Expand => 8.0,
        };

        let scroll_position = Vec2::new(scroll_position.offset_x, scroll_position.offset_y);
        let position = computed_node.size - Vec2::splat(bar_size) + scroll_position;
        let animation = match scrollbar.bar_type() {
            ScrollbarType::Horizontal => {
                KeaAnimation::new_with_clips(vec![
                    KeaAnimationClip::new::<Node>("top", Val::Px(position.y), 0.5),
                    KeaAnimationClip::new::<Node>("height", Val::Px(bar_size), 0.5),
                ])
            },
            ScrollbarType::Vertical => {
                KeaAnimation::new_with_clips(vec![
                    KeaAnimationClip::new::<Node>("left", Val::Px(position.x), 0.5),
                    KeaAnimationClip::new::<Node>("width", Val::Px(bar_size), 0.5),
                ])
            },
        };

        commands
            .entity(trigger.target())
            .insert(animation);

        break;
    }
}
