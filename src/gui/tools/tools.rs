use bevy::prelude::*;

#[derive(Component, Default)]
#[require(
    Node = Node {
        flex_direction: FlexDirection::Column,
        row_gap: Val::Px(kea::style::properties::ROW_GAP),
        width: Val::Percent(100.0),
        ..default()
    },
)]
pub(super) struct Tools;
