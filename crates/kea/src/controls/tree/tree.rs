use bevy::{
    prelude::*,
    ui::ContentSize,
};
use crate::{
    controls::{
        expander::KeaExpander,
        image::KeaImageNode,
        label::KeaLabel,
    },
    observers::KeaObservers,
    style,
};
use super::systems::{
    on_expander_event,
    on_tree_update,
    on_tree_children_added,
    on_tree_children_removed,
    on_tree_over,
    on_tree_out,
    on_tree_click,
};

#[derive(Component)]
#[require(
    KeaObservers<Self> = Self::observers(),
    Node,
)]
pub struct KeaTree {
    pub(super) data_item_block: Entity,
    pub(super) data_item_image: Entity,
    pub(super) contents: Entity,
    pub(super) selected: bool,
}

impl KeaTree {
    /// The items bundle should be a SpawnRelated bundle through the 'children!' macro.
    pub fn bundle(header: impl Bundle, items: impl Bundle) -> impl Bundle {(
        KeaExpander::bundle(DataItem::bundle(header),
        (
            KeaTreeContents,
            items,
        )),
        Self {
            data_item_block: Entity::PLACEHOLDER,
            data_item_image: Entity::PLACEHOLDER,
            contents: Entity::PLACEHOLDER,
            selected: false,
        },
    )}

    pub fn bundle_with_label(label: &str, items: impl Bundle) -> impl Bundle {(
        Self::bundle(KeaLabel::bundle(label), items),
    )}

    fn observers() -> KeaObservers<Self> {
        KeaObservers::new(vec![
            Observer::new(on_expander_event),
            Observer::new(on_tree_update),
        ])
    }
}

/// The root point of the control. Should be a sibling of KeaExpander.
#[derive(Component)]
#[require(
    KeaObservers<Self> = Self::observers(),
    Node = Self::node(),
)]
pub struct KeaTreeContents;

impl KeaTreeContents {
    fn observers() -> KeaObservers<Self> {
        KeaObservers::new(vec![
            Observer::new(on_tree_children_added),
            Observer::new(on_tree_children_removed),
        ])
    }

    fn node() -> Node {
        Node {
            left: Val::Px(style::properties::FONT_SIZE),
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(style::properties::ROW_GAP),
            width: Val::Percent(100.0),
            ..default()
        }
    }
}

#[derive(Component, Default)]
pub(super) enum DataItemContent {
    #[default]
    Block,
    Image,
}

#[derive(Component)]
#[require(
    Node = Self::node(),
    KeaObservers<Self> = Self::observers(),
)]
pub(super) struct DataItem;

impl DataItem {
    fn bundle(contents: impl Bundle) -> impl Bundle {(
        Self,
        children![
            (
                Node::default(),
                ContentSize::fixed_size(Vec2::splat(style::properties::FONT_SIZE)),
                Pickable::IGNORE,
                DataItemContent::Block,
            ),
            (
                Node {
                    display: Display::None,
                    ..default()
                },
                KeaImageNode(format!("kea://icons/expander.svg#image{size}x{size}", size = style::properties::FONT_SIZE)),
                Pickable::IGNORE,
                DataItemContent::Image,
            ),
            (
                contents,
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            align_items: AlignItems::Center,
            width: Val::Percent(100.0),
            ..default()
        }
    }

    fn observers() -> KeaObservers<Self> {
        KeaObservers::new(vec![
            Observer::new(on_tree_over),
            Observer::new(on_tree_out),
            Observer::new(on_tree_click),
        ])
    }
}
