use bevy::{
    prelude::*,
    render::camera::NormalizedRenderTarget,
    ui::RelativeCursorPosition,
    window::SystemCursorIcon,
    winit::cursor::CursorIcon,
};
use crate::{
    constants,
    mouse::ChangeMouseCursor,
    observers::KeaObservers,
    style,
};
use super::anchors::KeaAnchors;

///
/// KeaSizerTarget
///
/// Determines which node to resize.
///
pub enum KeaSizerTarget {
    Parent,
    Entity(Entity),
}

///
/// KeaSizer
///
/// Control that allows a parent node's size to be modified.
///
#[derive(Component)]
#[require(
    Node = Self::node(),
    Pickable = Self::pickable(),
    RelativeCursorPosition,
    ZIndex(constants::SIZER_Z_INDEX),
)]
pub struct KeaSizer {
    anchors: KeaAnchors,
    drag_anchor: KeaAnchors,
    pending_drag_anchor: KeaAnchors,
    target: KeaSizerTarget,
}

impl KeaSizer {
    const OFFSET: f32 = style::properties::SIZER_SIZE * 0.5;

    pub fn bundle(anchors: KeaAnchors) -> impl Bundle {
        Self::internal_bundle(anchors, KeaSizerTarget::Parent)
    }

    pub fn bundle_with_target(anchors: KeaAnchors, target: Entity) -> impl Bundle {
        Self::internal_bundle(anchors, KeaSizerTarget::Entity(target))
    }

    fn internal_bundle(anchors: KeaAnchors, target: KeaSizerTarget) -> impl Bundle {(
        Self {
            anchors,
            drag_anchor: KeaAnchors::empty(),
            pending_drag_anchor: KeaAnchors::empty(),
            target,
        },
        KeaObservers::new(vec![
            Observer::new(on_move),
            Observer::new(on_out),
            Observer::new(on_down),
            Observer::new(on_drag),
        ])
    )}

    fn node() -> Node {
        Node {
            position_type: PositionType::Absolute,
            ..default()
        }
    }

    fn pickable() -> Pickable {
        Pickable {
            should_block_lower: false,
            is_hoverable: true
        }
    }
}

#[derive(Resource)]
struct SizerState {
    pressed_sizer: Option<Entity>,
}

pub(super) fn build(app: &mut App) {
    app
        .insert_resource(SizerState {
            pressed_sizer: None,
        })
        .add_observer(on_add)
        .add_observer(on_up)
        .add_observer(on_remove_node);
}

fn px(val: &Val) -> Option<f32> {
    match val {
        Val::Px(result) => Some(*result),
        _ => None,
    }
}

fn node_size(node: &Node) -> Option<Vec2> {
    let Some(width) = px(&node.width) else {
        return None;
    };

    let Some(height) = px(&node.height) else {
        return None;
    };

    Some(Vec2::new(width, height))
}

fn from_normalized(node: &Node, position: Vec2) -> Option<Vec2> {
    let Some(node_size) = node_size(&node) else {
        return None;
    };

    Some(Vec2::new(position.x * node_size.x, position.y * node_size.y))
}

fn from_relative_position(node: &Node, relative_position: &RelativeCursorPosition) -> Option<Vec2> {
    let Some(position) = relative_position.normalized else {
        return None;
    };

    from_normalized(node, position)
}

fn node_position(node: &Node) -> Option<Vec2> {
    let Some(left) = px(&node.left) else {
        return None;
    };

    let Some(top) = px(&node.top) else {
        return None;
    };

    Some(Vec2::new(left, top))
}

fn sync_position(sizer: &KeaSizer, sizer_node: &mut Node, target_node: &Node) {
    match sizer.target {
        KeaSizerTarget::Parent => {
            sizer_node.left = Val::Px(-KeaSizer::OFFSET);
            sizer_node.top = Val::Px(-KeaSizer::OFFSET);
        },
        KeaSizerTarget::Entity(_) => {
            let x = if let Val::Px(value) = target_node.left { value } else { 0.0 };
            let y = if let Val::Px(value) = target_node.top { value } else { 0.0 };

            sizer_node.left = Val::Px(x - KeaSizer::OFFSET);
            sizer_node.top = Val::Px(y - KeaSizer::OFFSET);
        }
    }
}

fn to_cursor_icon(anchor: KeaAnchors) -> CursorIcon {
    let result = match anchor {
        KeaAnchors::TOP | KeaAnchors::BOTTOM => SystemCursorIcon::NsResize,
        KeaAnchors::LEFT | KeaAnchors::RIGHT => SystemCursorIcon::EwResize,
        KeaAnchors::TOPLEFT | KeaAnchors::BOTTOMRIGHT => SystemCursorIcon::NwseResize,
        KeaAnchors::TOPRIGHT | KeaAnchors::BOTTOMLEFT => SystemCursorIcon::NeswResize,
        _ => SystemCursorIcon::Default,
    };

    result.into()
}

fn update_sizer_and_cursor(
    entity: Entity,
    window: Entity,
    sizers: &mut Query<(&mut KeaSizer, &Node, &RelativeCursorPosition)>,
    commands: &mut Commands,
) -> bool {
    let Ok((mut sizer, node, relative_position)) = sizers.get_mut(entity) else {
        return false;
    };

    let Some(node_size) = node_size(&node) else {
        return false;
    };

    let Some(position) = from_relative_position(&node, &relative_position) else {
        return false;
    };

    let last_pending = sizer.pending_drag_anchor;

    let (_, anchor) = KeaAnchors::bounds_from_position(node_size, position);
    if anchor != last_pending {
        if !anchor.is_empty() && sizer.anchors.contains(anchor) {
            sizer.pending_drag_anchor = anchor;
        } else {
            sizer.pending_drag_anchor = KeaAnchors::empty();
        };

        commands.trigger(ChangeMouseCursor::Set {
            window,
            cursor: to_cursor_icon(sizer.pending_drag_anchor),
        });
    }

    true
}

fn on_move(
    trigger: Trigger<Pointer<Move>>,
    state: Res<SizerState>,
    mut sizers: Query<(&mut KeaSizer, &Node, &RelativeCursorPosition)>,
    mut commands: Commands,
) {
    // A sizer is currently being dragged.
    if state.pressed_sizer.is_some() {
        return;
    }

    if let NormalizedRenderTarget::Window(window) = trigger.pointer_location.target {
        update_sizer_and_cursor(trigger.target(), window.entity(), &mut sizers, &mut commands);
    }
}

fn on_out(
    trigger: Trigger<Pointer<Out>>,
    state: Res<SizerState>,
    mut sizers: Query<&mut KeaSizer>,
    mut commands: Commands,
) {
    let Ok(mut sizer) = sizers.get_mut(trigger.target()) else {
        return;
    };

    if state.pressed_sizer.is_some() {
        return;
    }

    sizer.pending_drag_anchor = KeaAnchors::empty();

    if let NormalizedRenderTarget::Window(window) = trigger.pointer_location.target {
        commands.trigger(ChangeMouseCursor::Set {
            window: window.entity(),
            cursor: to_cursor_icon(sizer.pending_drag_anchor),
        });
    }
}

fn on_add(
    trigger: Trigger<OnAdd, KeaSizer>,
    sizers: Query<&KeaSizer>,
    children: Query<&ChildOf>,
    mut nodes: Query<&mut Node>,
) {
    let Ok(sizer) = sizers.get(trigger.target()) else {
        return;
    };

    let entities = match sizer.target {
        KeaSizerTarget::Parent => {
            let Ok(child) = children.get(trigger.target()) else {
                return;
            };

            [trigger.target(), child.parent()]
        },
        KeaSizerTarget::Entity(target) => [trigger.target(), target],
    };

    let Ok([mut sizer_node, target_node]) = nodes.get_many_mut(entities) else {
        return;
    };

    let Some(node_size) = node_size(&target_node) else {
        return;
    };

    sync_position(sizer, &mut sizer_node, &target_node);

    sizer_node.width = Val::Px(node_size.x + KeaSizer::OFFSET * 2.0);
    sizer_node.height = Val::Px(node_size.y + KeaSizer::OFFSET * 2.0);
}

fn on_remove_node(
    trigger: Trigger<OnRemove, Node>,
    sizers: Query<(&KeaSizer, Entity)>,
    mut commands: Commands,
) {
    if sizers.contains(trigger.target()) {
        return;
    }

    let mut entity = Entity::PLACEHOLDER;
    for (sizer, sizer_entity) in sizers {
        let result = match sizer.target {
            KeaSizerTarget::Parent => Entity::PLACEHOLDER,
            KeaSizerTarget::Entity(entity) => {
                if trigger.target() == entity {
                    sizer_entity
                } else {
                    Entity::PLACEHOLDER
                }
            }
        };

        if result != Entity::PLACEHOLDER {
            entity = result;
            break;
        }
    }

    if entity != Entity::PLACEHOLDER {
        commands.entity(entity).despawn();
    }
}

fn on_down(
    mut trigger: Trigger<Pointer<Pressed>>,
    mut sizers: Query<&mut KeaSizer>,
    mut state: ResMut<SizerState>,
    mut commands: Commands,
) {
    let Ok(mut sizer) = sizers.get_mut(trigger.target()) else {
        return;
    };

    if !sizer.pending_drag_anchor.is_empty() {
        sizer.drag_anchor = sizer.pending_drag_anchor;
        state.pressed_sizer = Some(trigger.target());
        trigger.propagate(false);

        if let NormalizedRenderTarget::Window(window) = trigger.pointer_location.target {
            commands.trigger(ChangeMouseCursor::Lock {
                window: window.entity(),
                target: trigger.target(),
            });
        }
    }
}

/// This is a global observer. Needed if the pointer is off of the target sizer.
fn on_up(
    trigger: Trigger<Pointer<Released>>,
    mut sizers: Query<(&mut KeaSizer, &Node, &RelativeCursorPosition)>,
    mut state: ResMut<SizerState>,
    mut commands: Commands,
) {
    let Some(sizer_entity) = state.pressed_sizer else {
        return;
    };

    state.pressed_sizer = None;

    if let NormalizedRenderTarget::Window(window) = trigger.pointer_location.target {
        commands.trigger(ChangeMouseCursor::Unlock {
            window: window.entity(),
            target: sizer_entity,
        });

        update_sizer_and_cursor(sizer_entity, window.entity(), &mut sizers, &mut commands);
    }

    let Ok((mut sizer, _, _)) = sizers.get_mut(sizer_entity) else {
        return;
    };

    sizer.drag_anchor = KeaAnchors::empty();
}

fn on_drag(
    trigger: Trigger<Pointer<Drag>>,
    sizers: Query<&KeaSizer>,
    children: Query<&ChildOf>,
    mut nodes: Query<&mut Node>,
) {
    let Ok(sizer) = sizers.get(trigger.target) else {
        return;
    };

    let entities = match sizer.target {
        KeaSizerTarget::Parent => {
            let Ok(child) = children.get(trigger.target) else {
                return;
            };

            [trigger.target, child.parent()]
        },
        KeaSizerTarget::Entity(entity) => [trigger.target, entity],
    };

    let Ok([mut node, mut target_node]) = nodes.get_many_mut(entities) else {
        return;
    };

    let Some(size) = node_size(&node) else {
        return;
    };

    let Some(target_node_position) = node_position(&target_node) else {
        return;
    };

    let Some(target_node_size) = node_size(&target_node) else {
        return;
    };

    let delta = trigger.delta;

    if sizer.drag_anchor.contains(KeaAnchors::LEFT) {
        node.width = Val::Px(size.x - delta.x);

        target_node.left = Val::Px(target_node_position.x + delta.x);
        target_node.width = Val::Px(target_node_size.x - delta.x);
    }

    if sizer.drag_anchor.contains(KeaAnchors::TOP) {
        node.height = Val::Px(size.y - delta.y);

        target_node.top = Val::Px(target_node_position.y + delta.y);
        target_node.height = Val::Px(target_node_size.y - delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::RIGHT) {
        node.width = Val::Px(size.x + delta.x);

        target_node.width = Val::Px(target_node_size.x + delta.x);
    }

    if sizer.drag_anchor.contains(KeaAnchors::BOTTOM) {
        node.height = Val::Px(size.y + delta.y);

        target_node.height = Val::Px(target_node_size.y + delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::TOPLEFT) {
        node.width = Val::Px(size.x - delta.x);
        node.height = Val::Px(size.y - delta.y);

        target_node.left = Val::Px(target_node_position.x + delta.x);
        target_node.top = Val::Px(target_node_position.y + delta.y);
        target_node.width = Val::Px(target_node_size.x - delta.x);
        target_node.height = Val::Px(target_node_size.y - delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::TOPRIGHT) {
        node.width = Val::Px(size.x + delta.x);
        node.height = Val::Px(size.y - delta.y);

        target_node.top = Val::Px(target_node_position.y + delta.y);
        target_node.width = Val::Px(target_node_size.x + delta.x);
        target_node.height = Val::Px(target_node_size.y - delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::BOTTOMRIGHT) {
        node.width = Val::Px(size.x + delta.x);
        node.height = Val::Px(size.y + delta.y);

        target_node.width = Val::Px(target_node_size.x + delta.x);
        target_node.height = Val::Px(target_node_size.y + delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::BOTTOMLEFT) {
        node.width = Val::Px(size.x - delta.x);
        node.height = Val::Px(size.y + delta.y);

        target_node.left = Val::Px(target_node_position.x + delta.x);
        target_node.width = Val::Px(target_node_size.x - delta.x);
        target_node.height = Val::Px(target_node_size.y + delta.y);
    }

    sync_position(sizer, &mut node, &target_node);
}
