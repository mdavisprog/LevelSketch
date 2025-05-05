use bevy::{
    prelude::*,
    ui::RelativeCursorPosition,
};
use bitflags;
use super::style;

bitflags::bitflags! {
    #[derive(Clone, Copy, Debug)]
    pub struct Anchors: u16 {
        const LEFT = 1 << 0;
        const TOP = 1 << 1;
        const RIGHT = 1 << 2;
        const BOTTOM = 1 << 3;
        const TOPLEFT = 1 << 4;
        const TOPRIGHT = 1 << 5;
        const BOTTOMLEFT = 1 << 6;
        const BOTTOMRIGHT = 1 << 7;
    }
}

impl Anchors {
    fn bounds_from_position(size: Vec2, position: Vec2) -> (Rect, Anchors) {
        let anchors = Self::to_array();

        for anchor in anchors {
            if anchor.bounds(size).contains(position) {
                return (anchor.bounds(size), anchor);
            }
        }

        (Rect::EMPTY, Anchors::empty())
    }

    fn bounds(&self, size: Vec2) -> Rect {
        let padding = style::properties::SIZER_SIZE;

        if self.contains(Anchors::LEFT) {
            Rect::new(
                0.0,
                0.0,
                padding,
                size.y,
            )
        } else if self.contains(Anchors::TOP) {
            Rect::new(
                0.0,
                0.0,
                size.x,
                padding,
            )
        } else if self.contains(Anchors::RIGHT) {
            Rect::new(
                size.x - padding,
                0.0,
                size.x,
                size.y,
            )
        } else if self.contains(Anchors::BOTTOM) {
            Rect::new(
                0.0,
                size.y - padding,
                size.x,
                size.y,
            )
        } else if self.contains(Anchors::TOPLEFT) {
            Rect::new(
                0.0,
                0.0,
                padding * 2.0,
                padding * 2.0,
            )
        } else if self.contains(Anchors::TOPRIGHT) {
            Rect::new(
                size.x - padding * 2.0,
                0.0,
                size.x,
                padding * 2.0,
            )
        } else if self.contains(Anchors::BOTTOMRIGHT) {
            Rect::new(
                size.x - padding * 2.0,
                size.y - padding * 2.0,
                size.x,
                size.y,
            )
        } else if self.contains(Anchors::BOTTOMLEFT) {
            Rect::new(
                0.0,
                size.y - padding * 2.0,
                padding * 2.0,
                size.y,
            )
        } else {
            Rect::EMPTY
        }
    }

    fn to_array() -> [Anchors; 8] {[
        Anchors::TOPLEFT,
        Anchors::TOPRIGHT,
        Anchors::BOTTOMRIGHT,
        Anchors::BOTTOMLEFT,
        Anchors::LEFT,
        Anchors::TOP,
        Anchors::RIGHT,
        Anchors::BOTTOM,
    ]}
}

#[derive(Component)]
#[require(
    Node = Self::node(),
    Pickable = Self::pickable(),
    ZIndex(i32::MAX),
    RelativeCursorPosition,
)]
pub struct Sizer {
    anchors: Anchors,
    drag_anchor: Anchors,
}

impl Sizer {
    pub fn new(anchors: Anchors) -> Self {
        Self {
            anchors,
            drag_anchor: Anchors::empty(),
        }
    }

    ///
    /// Need to listen for Parent component insertions, which unfortunately occurs after
    /// the Sizer component is added. There currently is work being done on setting up
    /// the hierarchy before the OnAdd trigger is invoked for the desired component, so
    /// this function can be revisited when that feature is implemented.
    /// 
    pub fn on_added(
        trigger: Trigger<OnAdd, ChildOf>,
        sizers: Query<(Entity, &ChildOf), With<Self>>,
        mut nodes: Query<&mut Node>,
        mut commands: Commands,
    ) {
        let Ok((sizer, child_of)) = sizers.get(trigger.target()) else {
            return;
        };

        let Ok([mut sizer_node, parent_node]) = nodes.get_many_mut([trigger.target(), child_of.parent()]) else {
            return;
        };

        let Some(node_size) = Self::node_size(&parent_node) else {
            return;
        };

        let overflow = style::properties::SIZER_SIZE * 0.5;
        sizer_node.left = Val::Px(-overflow);
        sizer_node.top = Val::Px(-overflow);
        sizer_node.width = Val::Px(node_size.x + overflow * 2.0);
        sizer_node.height = Val::Px(node_size.y + overflow * 2.0);

        commands
            .entity(sizer)
            .observe(Self::on_down)
            .observe(Self::on_up)
            .observe(Self::on_drag);
    }

    fn on_down(
        mut trigger: Trigger<Pointer<Pressed>>,
        mut sizers: Query<(&mut Sizer, &Node, &RelativeCursorPosition)>,
    ) {
        let Ok((mut sizer, node, relative_position)) = sizers.get_mut(trigger.target) else {
            return;
        };

        let Some(node_size) = Self::node_size(&node) else {
            return;
        };

        let Some(position) = Self::from_relative_position(&node, &relative_position) else {
            return;
        };

        let (_, anchor) = Anchors::bounds_from_position(node_size, position);
        if !anchor.is_empty() && sizer.anchors.contains(anchor) {
            sizer.drag_anchor = anchor;
            trigger.propagate(false);
        }
    }

    fn on_up(
        trigger: Trigger<Pointer<Released>>,
        mut sizers: Query<&mut Sizer>,
    ) {
        let Ok(mut sizer) = sizers.get_mut(trigger.target) else {
            return;
        };

        sizer.drag_anchor = Anchors::empty();
    }

    fn on_drag(
        trigger: Trigger<Pointer<Drag>>,
        sizers: Query<(&Sizer, &ChildOf)>,
        mut nodes: Query<&mut Node>,
    ) {
        let Ok((sizer, child_of)) = sizers.get(trigger.target) else {
            return;
        };

        let Ok([mut node, mut parent_node]) = nodes.get_many_mut([trigger.target, child_of.parent()]) else {
            return;
        };

        let Some(node_size) = Self::node_size(&node) else {
            return;
        };

        let Some(parent_node_position) = Self::node_position(&parent_node) else {
            return;
        };

        let Some(parent_node_size) = Self::node_size(&parent_node) else {
            return;
        };

        let delta = trigger.delta;

        if sizer.drag_anchor.contains(Anchors::LEFT) {
            node.width = Val::Px(node_size.x - delta.x);

            parent_node.left = Val::Px(parent_node_position.x + delta.x);
            parent_node.width = Val::Px(parent_node_size.x - delta.x);
        }

        if sizer.drag_anchor.contains(Anchors::TOP) {
            node.height = Val::Px(node_size.y - delta.y);

            parent_node.top = Val::Px(parent_node_position.y + delta.y);
            parent_node.height = Val::Px(parent_node_size.y - delta.y);
        }

        if sizer.drag_anchor.contains(Anchors::RIGHT) {
            node.width = Val::Px(node_size.x + delta.x);

            parent_node.width = Val::Px(parent_node_size.x + delta.x);
        }

        if sizer.drag_anchor.contains(Anchors::BOTTOM) {
            node.height = Val::Px(node_size.y + delta.y);

            parent_node.height = Val::Px(parent_node_size.y + delta.y);
        }

        if sizer.drag_anchor.contains(Anchors::TOPLEFT) {
            node.width = Val::Px(node_size.x - delta.x);
            node.height = Val::Px(node_size.y - delta.y);

            parent_node.left = Val::Px(parent_node_position.x + delta.x);
            parent_node.top = Val::Px(parent_node_position.y + delta.y);
            parent_node.width = Val::Px(parent_node_size.x - delta.x);
            parent_node.height = Val::Px(parent_node_size.y - delta.y);
        }

        if sizer.drag_anchor.contains(Anchors::TOPRIGHT) {
            node.width = Val::Px(node_size.x + delta.x);
            node.height = Val::Px(node_size.y - delta.y);

            parent_node.top = Val::Px(parent_node_position.y + delta.y);
            parent_node.width = Val::Px(parent_node_size.x + delta.x);
            parent_node.height = Val::Px(parent_node_size.y - delta.y);
        }

        if sizer.drag_anchor.contains(Anchors::BOTTOMRIGHT) {
            node.width = Val::Px(node_size.x + delta.x);
            node.height = Val::Px(node_size.y + delta.y);

            parent_node.width = Val::Px(parent_node_size.x + delta.x);
            parent_node.height = Val::Px(parent_node_size.y + delta.y);
        }

        if sizer.drag_anchor.contains(Anchors::BOTTOMLEFT) {
            node.width = Val::Px(node_size.x - delta.x);
            node.height = Val::Px(node_size.y + delta.y);

            parent_node.left = Val::Px(parent_node_position.x + delta.x);
            parent_node.width = Val::Px(parent_node_size.x - delta.x);
            parent_node.height = Val::Px(parent_node_size.y + delta.y);
        }
    }

    fn from_normalized(node: &Node, position: Vec2) -> Option<Vec2> {
        let Some(node_size) = Self::node_size(&node) else {
            return None;
        };

        Some(Vec2::new(position.x * node_size.x, position.y * node_size.y))
    }

    fn from_relative_position(node: &Node, relative_position: &RelativeCursorPosition) -> Option<Vec2> {
        let Some(position) = relative_position.normalized else {
            return None;
        };

        Self::from_normalized(node, position)
    }

    fn node_position(node: &Node) -> Option<Vec2> {
        let Some(left) = Self::px(&node.left) else {
            return None;
        };

        let Some(top) = Self::px(&node.top) else {
            return None;
        };

        Some(Vec2::new(left, top))
    }

    fn node_size(node: &Node) -> Option<Vec2> {
        let Some(width) = Self::px(&node.width) else {
            return None;
        };

        let Some(height) = Self::px(&node.height) else {
            return None;
        };

        Some(Vec2::new(width, height))
    }

    fn px(val: &Val) -> Option<f32> {
        match val {
            Val::Px(result) => Some(*result),
            _ => None,
        }
    }

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
