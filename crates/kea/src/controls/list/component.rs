use bevy::prelude::*;

pub enum KeaListBehavior {
    NoSelect,
    Select,
}

///
/// KeaList
///
/// Component that contains a list of child items and emits events when
/// interacting with the list.
///
#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct KeaList {
    pub behavior: KeaListBehavior,
    pub(super) selected: Vec<Entity>,
}

impl Default for KeaList {
    fn default() -> Self {
        Self {
            behavior: KeaListBehavior::Select,
            selected: Vec::new(),
        }
    }
}

impl KeaList {
    pub fn new(behavior: KeaListBehavior) -> impl Bundle {
        Self {
            behavior,
            ..default()
        }
    }

    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Column,
            width: Val::Percent(100.0),
            ..default()
        }
    }
}

///
/// KeaListLabelItems
///
/// Component that defines what labels should be added to a KeaList component.
/// This component is removed after items from the component are spawned for
/// the owning list.
///
#[derive(Component)]
pub struct KeaListLabelItems(pub Vec<String>);

///
/// KeaListItem
///
/// Marker component for KeaList items.
///
#[derive(Component)]
#[require(
    BackgroundColor(Color::NONE),
)]
pub(super) struct KeaListItem;
