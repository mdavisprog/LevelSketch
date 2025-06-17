use bevy::prelude::*;

///
/// KeaListSelect
///
/// Event emitted when a list item is selected.
///
#[derive(Event)]
pub struct KeaListSelect {
    /// The list item entity
    pub entity: Entity,

    /// Index into the children array
    pub index: usize,
}

#[derive(Event)]
pub struct KeaListHover {
    /// The list item entity
    pub entity: Entity,

    /// Index into the children array
    pub index: usize,
}
