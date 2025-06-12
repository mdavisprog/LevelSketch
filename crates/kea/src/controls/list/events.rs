use bevy::prelude::*;

///
/// KeaListSelect
///
/// Event emitted when a list item is selected.
///
#[derive(Event)]
pub struct KeaListSelect {
    pub entity: Entity,
}
