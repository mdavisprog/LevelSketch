use bevy::{
    prelude::*,
    ui::RelativeCursorPosition,
};
use crate::{
    observers::KeaObservers,
    style,
};
use super::anchors::KeaAnchors;

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
)]
pub struct KeaSizer {
    anchors: KeaAnchors,
    drag_anchor: KeaAnchors,
}

impl KeaSizer {
    pub fn bundle(anchors: KeaAnchors) -> impl Bundle {(
        Self {
            anchors,
            drag_anchor: KeaAnchors::empty(),
        },
        KeaObservers::new(vec![
            Observer::new(on_down),
            Observer::new(on_up),
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

pub(super) fn build(app: &mut App) {
    app.add_observer(on_add);
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

fn on_add(
    trigger: Trigger<OnAdd, KeaSizer>,
    sizers: Query<&ChildOf, With<KeaSizer>>,
    mut nodes: Query<&mut Node>,
) {
    let Ok(child_of) = sizers.get(trigger.target()) else {
        return;
    };

    let Ok([mut sizer_node, parent_node]) = nodes.get_many_mut([trigger.target(), child_of.parent()]) else {
        return;
    };

    let Some(node_size) = node_size(&parent_node) else {
        return;
    };

    let overflow = style::properties::SIZER_SIZE * 0.5;
    sizer_node.left = Val::Px(-overflow);
    sizer_node.top = Val::Px(-overflow);
    sizer_node.width = Val::Px(node_size.x + overflow * 2.0);
    sizer_node.height = Val::Px(node_size.y + overflow * 2.0);
}

fn on_down(
    mut trigger: Trigger<Pointer<Pressed>>,
    mut sizers: Query<(&mut KeaSizer, &Node, &RelativeCursorPosition)>,
) {
    let Ok((mut sizer, node, relative_position)) = sizers.get_mut(trigger.target) else {
        return;
    };

    let Some(node_size) = node_size(&node) else {
        return;
    };

    let Some(position) = from_relative_position(&node, &relative_position) else {
        return;
    };

    let (_, anchor) = KeaAnchors::bounds_from_position(node_size, position);
    if !anchor.is_empty() && sizer.anchors.contains(anchor) {
        sizer.drag_anchor = anchor;
        trigger.propagate(false);
    }
}

fn on_up(
    trigger: Trigger<Pointer<Released>>,
    mut sizers: Query<&mut KeaSizer>,
) {
    let Ok(mut sizer) = sizers.get_mut(trigger.target) else {
        return;
    };

    sizer.drag_anchor = KeaAnchors::empty();
}

fn on_drag(
    trigger: Trigger<Pointer<Drag>>,
    sizers: Query<(&KeaSizer, &ChildOf)>,
    mut nodes: Query<&mut Node>,
) {
    let Ok((sizer, child_of)) = sizers.get(trigger.target) else {
        return;
    };

    let Ok([mut node, mut parent_node]) = nodes.get_many_mut([trigger.target, child_of.parent()]) else {
        return;
    };

    let Some(size) = node_size(&node) else {
        return;
    };

    let Some(parent_node_position) = node_position(&parent_node) else {
        return;
    };

    let Some(parent_node_size) = node_size(&parent_node) else {
        return;
    };

    let delta = trigger.delta;

    if sizer.drag_anchor.contains(KeaAnchors::LEFT) {
        node.width = Val::Px(size.x - delta.x);

        parent_node.left = Val::Px(parent_node_position.x + delta.x);
        parent_node.width = Val::Px(parent_node_size.x - delta.x);
    }

    if sizer.drag_anchor.contains(KeaAnchors::TOP) {
        node.height = Val::Px(size.y - delta.y);

        parent_node.top = Val::Px(parent_node_position.y + delta.y);
        parent_node.height = Val::Px(parent_node_size.y - delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::RIGHT) {
        node.width = Val::Px(size.x + delta.x);

        parent_node.width = Val::Px(parent_node_size.x + delta.x);
    }

    if sizer.drag_anchor.contains(KeaAnchors::BOTTOM) {
        node.height = Val::Px(size.y + delta.y);

        parent_node.height = Val::Px(parent_node_size.y + delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::TOPLEFT) {
        node.width = Val::Px(size.x - delta.x);
        node.height = Val::Px(size.y - delta.y);

        parent_node.left = Val::Px(parent_node_position.x + delta.x);
        parent_node.top = Val::Px(parent_node_position.y + delta.y);
        parent_node.width = Val::Px(parent_node_size.x - delta.x);
        parent_node.height = Val::Px(parent_node_size.y - delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::TOPRIGHT) {
        node.width = Val::Px(size.x + delta.x);
        node.height = Val::Px(size.y - delta.y);

        parent_node.top = Val::Px(parent_node_position.y + delta.y);
        parent_node.width = Val::Px(parent_node_size.x + delta.x);
        parent_node.height = Val::Px(parent_node_size.y - delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::BOTTOMRIGHT) {
        node.width = Val::Px(size.x + delta.x);
        node.height = Val::Px(size.y + delta.y);

        parent_node.width = Val::Px(parent_node_size.x + delta.x);
        parent_node.height = Val::Px(parent_node_size.y + delta.y);
    }

    if sizer.drag_anchor.contains(KeaAnchors::BOTTOMLEFT) {
        node.width = Val::Px(size.x - delta.x);
        node.height = Val::Px(size.y + delta.y);

        parent_node.left = Val::Px(parent_node_position.x + delta.x);
        parent_node.width = Val::Px(parent_node_size.x - delta.x);
        parent_node.height = Val::Px(parent_node_size.y + delta.y);
    }
}
