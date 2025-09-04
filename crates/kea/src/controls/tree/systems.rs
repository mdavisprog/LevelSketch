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
    style,
};
use super::{
    events::{
        KeaTreeClick,
        KeaTreeHover,
    },
    tree::{
        KeaTree,
        KeaTreeContents,
        DataItemContent,
    },
};

#[derive(Event)]
pub(super) struct UpdateTree;

pub(super) fn build(app: &mut App) {
    app
        .add_observer(on_add_tree_contents)
        .add_observer(on_add_data_item);
}

fn on_add_tree_contents(
    trigger: Trigger<OnAdd, KeaTreeContents>,
    children: Query<&ChildOf>,
    mut trees: Query<&mut KeaTree>,
) {
    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(mut tree) = trees.get_mut(parent) else {
            continue;
        };

        tree.contents = trigger.target();
        break;
    }
}

fn on_add_data_item(
    trigger: Trigger<OnAdd, DataItemContent>,
    contents: Query<&DataItemContent>,
    children: Query<&ChildOf>,
    mut trees: Query<&mut KeaTree>,
) {
    let Ok(content) = contents.get(trigger.target()) else {
        panic!("Failed to find DataItemContent component.");
    };

    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(mut tree) = trees.get_mut(parent) else {
            continue;
        };

        match content {
            DataItemContent::Block => {
                tree.data_item_block = trigger.target();
            },
            DataItemContent::Image => {
                tree.data_item_image = trigger.target();
            },
        }
        break;
    }
}

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

pub(super) fn on_tree_update(
    trigger: Trigger<UpdateTree>,
    trees: Query<&KeaTree>,
    parents: Query<&Children>,
    mut nodes: Query<&mut Node>,
) {
    let Ok(tree) = trees.get(trigger.target()) else {
        return;
    };

    let has_children = if let Ok(parent) = parents.get(tree.contents) {
        !parent.is_empty()
    } else {
        false
    };

    let entities = [tree.data_item_block, tree.data_item_image];
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
}

pub(super) fn on_tree_children_added(
    trigger: Trigger<OnAdd, Children>,
    children: Query<&ChildOf>,
    trees: Query<&KeaTree>,
    mut commands: Commands,
) {
    for parent in children.iter_ancestors(trigger.target()) {
        if !trees.contains(parent) {
            continue;
        }

        commands.trigger_targets(UpdateTree, parent);
        break;
    }
}

pub(super) fn on_tree_children_removed(
    trigger: Trigger<OnRemove, Children>,
    children: Query<&ChildOf>,
    trees: Query<&KeaTree>,
    mut commands: Commands,
) {
    for parent in children.iter_ancestors(trigger.target()) {
        if !trees.contains(parent) {
            continue;
        }

        commands.trigger_targets(UpdateTree, parent);
        break;
    }
}

pub(super) fn on_tree_over(
    trigger: Trigger<Pointer<Over>>,
    children: Query<&ChildOf>,
    trees: Query<&KeaTree>,
    mut commands: Commands,
) {
    for parent in children.iter_ancestors(trigger.target()) {
        if !trees.contains(parent) {
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
    trees: Query<&KeaTree>,
    mut commands: Commands,
) {
    let mut revert = true;
    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(tree) = trees.get(parent) else {
            continue;
        };

        revert = !tree.selected;

        commands.trigger_targets(KeaTreeHover {
            hovered: false,
        }, parent);
        break;
    }

    if revert {
        commands
            .entity(trigger.target())
            .insert(BackgroundColor(Color::NONE));
    }
}

pub(super) fn on_tree_click(
    trigger: Trigger<Pointer<Click>>,
    children: Query<&ChildOf>,
    trees: Query<&KeaTree>,
    mut commands: Commands,
) {
    for parent in children.iter_ancestors(trigger.target()) {
        if !trees.contains(parent) {
            continue;
        }

        commands.trigger_targets(KeaTreeClick, parent);
        break;
    }
}
