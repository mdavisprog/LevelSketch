use bevy::{
    input::mouse::MouseScrollUnit,
    prelude::*,
};
use crate::{
    controls::{
        anchors::KeaAnchors,
        button::KeaButtonClick,
        panel::KeaPanel,
        sizer::KeaSizer,
    },
};

pub(super) fn build(app: &mut App) {
    app
        .add_observer(on_add);
}

fn on_add(
    trigger: Trigger<OnAdd, KeaPanel>,
    mut commands: Commands,
) {
    commands.spawn(
        KeaSizer::bundle_with_target(KeaAnchors::all(), trigger.target())
    );
}

pub(super) fn on_close(
    trigger: Trigger<KeaButtonClick>,
    child: Query<&ChildOf>,
    mut commands: Commands,
) {
    let root = child.root_ancestor(trigger.target());

    commands
        .entity(root)
        .try_despawn();
}

pub(super) fn on_header_drag(
    trigger: Trigger<Pointer<Drag>>,
    children: Query<&ChildOf>,
    mut panels: Query<&mut Node, With<KeaPanel>>,
) {
    let Ok(child) = children.get(trigger.target) else {
        return;
    };

    let Ok(mut panel) = panels.get_mut(child.parent()) else {
        return;
    };

    let left = match panel.left {
        Val::Px(value) => value,
        _ => 0.0,
    };

    let top = match panel.top {
        Val::Px(value) => value,
        _ => 0.0,
    };

    let delta = trigger.delta;
    panel.left = Val::Px(left + delta.x);
    panel.top = Val::Px(top + delta.y);
}

pub(super) fn on_scroll(
    trigger: Trigger<Pointer<Scroll>>,
    mut scroll_positions: Query<&mut ScrollPosition>,
) {
    let Ok(mut scroll_position) = scroll_positions.get_mut(trigger.target()) else {
        return;
    };

    let scalar = match trigger.event().unit {
        MouseScrollUnit::Line => 12.0,
        MouseScrollUnit::Pixel => 1.0,
    };

    scroll_position.offset_x -= trigger.event().x * scalar;
    scroll_position.offset_y -= trigger.event().y * scalar;
}
