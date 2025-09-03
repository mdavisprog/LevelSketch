use bevy::prelude::*;
use crate::{
    animation::{
        KeaAnimation,
        KeaAnimationClip,
    },
    controls::expander::{
        KeaExpander,
        KeaExpanderEvent,
    },
    ready::KeaOnReady,
    style,
};
use super::{
    events::KeaTreeHover,
    tree::{
        KeaTree,
        KeaTreeRoot,
        DataItem,
        DataItemContent,
    },
};

#[derive(Event)]
pub(super) struct UpdateTree;

pub(super) fn on_expander_event(
    trigger: Trigger<KeaExpanderEvent>,
    expanders: Query<&KeaExpander>,
    parents: Query<&Children>,
    images: Query<&ImageNode>,
    mut commands: Commands,
) {
    let Ok(expander) = expanders.get(trigger.target()) else {
        return;
    };

    for child in parents.iter_descendants(trigger.target()) {
        if !images.contains(child) {
            continue;
        }

        let rotation = if expander.is_collapsed() {
            Quat::from_rotation_z(-90.0_f32.to_radians())
        } else {
            Quat::from_rotation_z(0.0)
        };

        commands
            .entity(child)
            .insert(KeaAnimation::new_with_clips(vec![
                KeaAnimationClip::new::<Transform>("rotation", rotation, 0.5),
            ]));

        break;
    }
}

pub(super) fn on_tree_ready(
    trigger: Trigger<KeaOnReady>,
    parents: Query<&Children>,
    data_items: Query<&DataItem>,
    data_item_contents: Query<&DataItemContent>,
    trees: Query<&KeaTree>,
    mut tree_roots: Query<&mut KeaTreeRoot>,
    mut commands: Commands,
) {
    let Ok(mut tree_root) = tree_roots.get_mut(trigger.target()) else {
        return;
    };

    // First pass is to find the entities for the data item block/image.
    for child in parents.iter_descendants_depth_first(trigger.target()) {
        if !data_items.contains(child) {
            continue;
        }

        for data_item_child in parents.iter_descendants(child) {
            if let Ok(data_item_content) = data_item_contents.get(data_item_child) {
                match data_item_content {
                    DataItemContent::Block => tree_root.data_item_block = data_item_child,
                    DataItemContent::Image => tree_root.data_item_image = data_item_child,
                }
            }
        }

        break;
    }

    // Next is to push the update for the tree to update the image based on if the tree
    // has any children.
    for child in parents.iter_descendants(trigger.target()) {
        if !trees.contains(child) {
            continue;
        }

        tree_root.tree = child;
        commands.trigger_targets(UpdateTree, child);

        break;
    }
}

pub(super) fn on_tree_update(
    trigger: Trigger<UpdateTree>,
    tree_roots: Query<&KeaTreeRoot>,
    parents: Query<&Children>,
    children: Query<&ChildOf>,
    mut nodes: Query<&mut Node>,
) {
    let has_children = if let Ok(parent) = parents.get(trigger.target()) {
        !parent.is_empty()
    } else {
        false
    };

    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(tree_root) = tree_roots.get(parent) else {
            continue;
        };

        let entities = [tree_root.data_item_block, tree_root.data_item_image];
        let Ok([mut block, mut image]) = nodes.get_many_mut(entities) else {
            return;
        };

        if has_children {
            block.display = Display::None;
            image.display = Display::Flex;
        } else {
            block.display = Display::Flex;
            image.display = Display::None;
        }

        break;
    }
}

pub(super) fn on_tree_children_added(
    trigger: Trigger<OnAdd, Children>,
    mut commands: Commands,
) {
    commands.trigger_targets(UpdateTree, trigger.target());
}

pub(super) fn on_tree_children_removed(
    trigger: Trigger<OnRemove, Children>,
    mut commands: Commands,
) {
    commands.trigger_targets(UpdateTree, trigger.target());
}

pub(super) fn on_tree_over(
    trigger: Trigger<Pointer<Over>>,
    children: Query<&ChildOf>,
    roots: Query<&KeaTreeRoot>,
    mut commands: Commands,
) {
    for parent in children.iter_ancestors(trigger.target()) {
        if !roots.contains(parent) {
            continue;
        }

        commands.trigger_targets(KeaTreeHover {
            hovered: true,
        }, parent);
        break;
    }

    commands
        .entity(trigger.target())
        .insert(BackgroundColor(style::colors::HIGHLIGHT));
}

pub(super) fn on_tree_out(
    trigger: Trigger<Pointer<Out>>,
    children: Query<&ChildOf>,
    roots: Query<&KeaTreeRoot>,
    mut commands: Commands,
) {
    for parent in children.iter_ancestors(trigger.target()) {
        if !roots.contains(parent) {
            continue;
        }

        commands.trigger_targets(KeaTreeHover {
            hovered: false,
        }, parent);
        break;
    }

    commands
        .entity(trigger.target())
        .insert(BackgroundColor(Color::NONE));
}
