use bevy::prelude::*;
use crate::{
    constants,
    controls::{
        anchors::KeaAnchors,
        button::KeaButtonClick,
        panel::KeaPanel,
        sizer::KeaSizer,
    },
};
use super::{
    component::KeaPanelCloseBehavior,
    KeaPanelClose,
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
        let Ok(panel) = panels.get(parent) else {
            continue;
        };

        match panel.close_behavior {
            KeaPanelCloseBehavior::Close => {
                commands
                    .entity(parent)
                    .try_despawn();
            },
            KeaPanelCloseBehavior::Trigger => {
                commands.trigger_targets(KeaPanelClose, parent);
            },
        }
        break;
    }
}

pub(super) fn on_header_pressed(
    trigger: Trigger<Pointer<Pressed>>,
    children: Query<&ChildOf>,
    panels: Query<Entity, With<KeaPanel>>,
    mut indices: Query<&mut ZIndex>,
) {
    let Ok(child) = children.get(trigger.target()) else {
        return;
    };

    for panel in panels {
        let Ok(mut index) = indices.get_mut(panel) else {
            continue;
        };

        if panel == child.parent() {
            index.0 = constants::PANEL_FOCUSED_Z_INDEX;
        } else {
            index.0 = constants::BASE_Z_INDEX;
        }
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
