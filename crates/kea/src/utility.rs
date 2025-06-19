use bevy::prelude::*;

pub(crate) fn is_descendant(target: Entity, parent: Entity, parents: &Query<&Children>) -> bool {
    for child in parents.iter_descendants_depth_first(parent) {
        if child == target {
            return true;
        }
    }

    false
}
