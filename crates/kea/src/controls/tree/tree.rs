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
    ready::KeaOnReadyComponent,
    style,
};
use super::systems::{
    on_expander_event,
    on_tree_ready,
    on_tree_update,
    on_tree_children_added,
    on_tree_children_removed,
    on_tree_over,
    on_tree_out,
    on_tree_click,
};

/// TODO: Make KeaTree the root entity and change KeaTreeRoot to KeaTreeContents.
/// KeaTree should hold all of the entities for quick access. API access for the
/// KeaTreeContents entity for quick access to get the tree children.
#[derive(Component)]
#[require(
    KeaObservers<Self> = Self::observers(),
    Node = Self::node(),
)]
pub struct KeaTree {
    _private: (),
}

impl KeaTree {
    /// The items bundle should be a SpawnRelated bundle through the 'children!' macro.
    pub fn bundle(header: impl Bundle, items: impl Bundle) -> impl Bundle {(
        KeaExpander::bundle(DataItem::bundle(header),
        (
            Self {
                _private: (),
            },
            items,
        )),
        KeaTreeRoot {
            data_item_block: Entity::PLACEHOLDER,
            data_item_image: Entity::PLACEHOLDER,
            tree: Entity::PLACEHOLDER,
        },
    )}

    pub fn bundle_with_label(label: &str, items: impl Bundle) -> impl Bundle {(
        Self::bundle(KeaLabel::bundle(label), items),
    )}

    fn observers() -> KeaObservers<Self> {
        KeaObservers::new(vec![
            Observer::new(on_tree_update),
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

/// The root point of the control. Should be a sibling of KeaExpander.
#[derive(Component)]
#[require(
    KeaOnReadyComponent,
    KeaObservers<Self> = Self::observers(),
)]
pub struct KeaTreeRoot {
    pub(super) data_item_block: Entity,
    pub(super) data_item_image: Entity,
    pub(super) tree: Entity,
}

impl KeaTreeRoot {
    fn observers() -> KeaObservers<Self> {
        KeaObservers::new(vec![
            Observer::new(on_expander_event),
            Observer::new(on_tree_ready),
        ])
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
