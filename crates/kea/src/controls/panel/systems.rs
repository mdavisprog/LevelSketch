use bevy::prelude::*;
use crate::{
    controls::{
        anchors::KeaAnchors,
        button::KeaButtonClick,
        panel::KeaPanel,
        sizer::KeaSizer,
    },
};

pub(super) fn build(app: &mut App) {
    app.add_observer(on_add);
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
    panels: Query<&KeaPanel>,
    mut commands: Commands,
) {
    for parent in child.iter_ancestors(trigger.target()) {
        if !panels.contains(parent) {
            continue;
        }

        commands
            .entity(parent)
            .try_despawn();
    }
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
