use bevy::prelude::*;
use crate::animation::{
    KeaAnimation,
    KeaAnimationClip,
    KeaAnimationComplete,
};
use super::{
    events::KeaExpanderEvent,
    expander::{
        Contents,
        KeaExpander,
        KeaExpanderState,
    },
};

pub(super) fn on_click_header(
    trigger: Trigger<Pointer<Click>>,
    children: Query<&ChildOf>,
    parents: Query<&Children>,
    contents: Query<&Contents>,
    computed_nodes: Query<&ComputedNode>,
    mut expanders: Query<&mut KeaExpander>,
    mut nodes: Query<&mut Node>,
    mut commands: Commands,
) {
    let Ok(child) = children.get(trigger.target()) else {
        return;
    };

    // The parent should be the expander component.
    let Ok(mut expander) = expanders.get_mut(child.parent()) else {
        return;
    };

    // Find the entity that has the Contents component.
    let mut contents_entity = Entity::PLACEHOLDER;
    for child in parents.iter_descendants(child.parent()) {
        if contents.contains(child) {
            contents_entity = child;
            break;
        }
    }

    let Ok(computed_node) = computed_nodes.get(contents_entity) else {
        return;
    };

    let node_height = computed_node.content_size.y;

    // Update the expander state.
    expander.state = match expander.state {
        KeaExpanderState::Expanded => {
            expander.expanded_height = node_height;
            KeaExpanderState::Collapsed
        },
        KeaExpanderState::Collapsed => KeaExpanderState::Expanded,
    };

    // If the expander is being collapsed, override the height value to be Val::Px so that
    // there is no visual artifact caused if starting value is Val::Percent.
    if let Ok(mut node) = nodes.get_mut(contents_entity) {
        if expander.state == KeaExpanderState::Collapsed {
            node.height = Val::Px(node_height);
        }
    };

    // Determine if position and height of the contents based on the state.
    let (top, height) = match expander.state {
        KeaExpanderState::Expanded => (Val::Px(0.0), Val::Px(expander.expanded_height)),
        KeaExpanderState::Collapsed => (Val::Px(-node_height), Val::Px(0.0)),
    };

    commands
        .entity(contents_entity)
        .insert(
            KeaAnimation::new_with_clips(vec![
                KeaAnimationClip::new::<Node>("top", top, 0.5),
                KeaAnimationClip::new::<Node>("height", height, 0.5),
            ]),
        );
    
    match expander.state {
        KeaExpanderState::Collapsed => {
            commands.trigger_targets(KeaExpanderEvent::Collapse, trigger.target());
        },
        KeaExpanderState::Expanded => {
            commands.trigger_targets(KeaExpanderEvent::Expand, trigger.target());
        },
    }
}

pub(super) fn on_contents_anim_complete(
    trigger: Trigger<KeaAnimationComplete>,
    expanders: Query<&KeaExpander>,
    children: Query<&ChildOf>,
    mut nodes: Query<&mut Node>,
) {
    let event = trigger.event();

    // The height value when fully expanded should be a Val::Percent(100.0). Unfortunately,
    // animating Val::Percent does not impact the computed node size so the height needs to
    // set manually. When the header is fully expanded and the animation completes, this
    // will restore the 'height' property of the 'Contents' entity to be Val::Percent(100.0).
    if event.field == "height" {
        let Ok(mut node) = nodes.get_mut(trigger.target()) else {
            return;
        };

        // Find the expander component entity and get the expanded state.
        for parent in children.iter_ancestors(trigger.target()) {
            let Ok(expander) = expanders.get(parent) else {
                continue;
            };

            if expander.state == KeaExpanderState::Expanded {
                node.height = Val::Percent(100.0);
            }

            break;
        }
    }
}

pub(super) fn on_expander_event(
    trigger: Trigger<KeaExpanderEvent>,
    parents: Query<&Children>,
    images: Query<&ImageNode>,
    mut commands: Commands,
) {
    let event = trigger.event();

    // Update the rotation of the expander icon.
    for child in parents.iter_descendants(trigger.target()) {
        if images.contains(child) {
            let rotation = match event {
                KeaExpanderEvent::Expand => Quat::from_rotation_z(0.0),
                KeaExpanderEvent::Collapse => Quat::from_rotation_z(180_f32.to_radians()),
            };

            commands
                .entity(child)
                .insert(KeaAnimation::new_with_clips(vec![
                    KeaAnimationClip::new::<Transform>("rotation", rotation, 0.5),
                ]));

            break;
        }
    }
}
