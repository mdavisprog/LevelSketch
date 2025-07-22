use bevy::prelude::*;

#[derive(Resource, Default)]
pub(super) struct KeaScrollableResource {
    pub active_scrollbar: Option<Entity>,
}
