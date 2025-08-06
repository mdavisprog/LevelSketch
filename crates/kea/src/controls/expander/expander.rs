use bevy::prelude::*;
use crate::{
    controls::{
        image::KeaImageNode,
        separator::KeaSeparator,
    },
    observers::KeaObservers,
    ready::KeaOnReadyComponent,
    style,
};
use super::systems::{
    on_click_header,
    on_contents_anim_complete,
    on_expander_event,
    on_ready,
};

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum KeaExpanderState {
    Expanded,
    Collapsed,
}

/// A KeaExpander provides a bundle where the contents can be collapsed or expanded. When
/// transitioning to a new state, an animation is played and the contents are properly clipped
/// if collapsed. This component provides a bundle with a labelled header and icon.
#[derive(Component)]
#[require(
    Node = Self::node(),
    KeaObservers<Self> = Self::observers(),
    KeaOnReadyComponent,
)]
pub struct KeaExpander {
    pub(super) state: KeaExpanderState,
    pub(super) expanded_height: f32,
    pub(super) contents_entity: Entity,
}

impl KeaExpander {
    pub fn bundle(header: impl Bundle, contents: impl Bundle) -> impl Bundle {(
        Self::new(),
        children![
            (
                header,
                KeaObservers::<Header>::new(vec![
                    Observer::new(on_click_header),
                ]),
            ),
            (
                Self::contents(contents),
            )
        ],
    )}

    pub fn bundle_with_header(label: &str, contents: impl Bundle) -> impl Bundle {(
        Self::new(),
        children![
            (
                Header::bundle(label),
                KeaObservers::<Header>::new(vec![
                    Observer::new(on_click_header),
                ]),
            ),
            (
                KeaSeparator::horizontal(),
            ),
            (
                Self::contents(contents),
            ),
        ],
    )}

    pub fn is_expanded(&self) -> bool {
        self.state == KeaExpanderState::Expanded
    }

    pub fn is_collapsed(&self) -> bool {
        self.state == KeaExpanderState::Collapsed
    }

    fn new() -> Self {
        Self {
            state: KeaExpanderState::Expanded,
            expanded_height: 0.0,
            contents_entity: Entity::PLACEHOLDER,
        }
    }

    fn contents(contents: impl Bundle) -> impl Bundle {(
        // Clips the entities with contents when being collapsed.
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            overflow: Overflow::clip(),
            ..default()
        },
        children![(
            Node {
                width: Val::Percent(100.0),
                height: Val::Percent(100.0),
                overflow: Overflow::scroll(),
                align_items: AlignItems::Start,
                ..default()
            },
            Contents::bundle(),
            children![(
                contents,
            )],
        )],
    )}

    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(style::properties::ROW_GAP),
            ..default()
        }
    }

    fn observers() -> KeaObservers<Self> {
        KeaObservers::new(vec![
            Observer::new(on_ready),
        ])
    }
}

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub(super) struct Header;

impl Header {
    fn bundle(label: &str) -> impl Bundle {(
        Self,
        KeaObservers::<Self>::new(vec![
            Observer::new(on_expander_event),
        ]),
        children![
            (
                Text::new(label),
                TextFont::from_font_size(style::properties::PANEL_HEADER_FONT_SIZE),
                TextLayout::new_with_justify(JustifyText::Center),
                Node {
                    width: Val::Percent(100.0),
                    ..default()
                },
                Pickable::IGNORE,
            ),
            (
                KeaImageNode(format!(
                    "kea://icons/expander.svg#image{size}x{size}",
                    size = style::properties::PANEL_HEADER_FONT_SIZE)
                ),
                Node {
                    align_self: AlignSelf::End,
                    ..default()
                },
                Pickable::IGNORE,
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            align_items: AlignItems::Center,
            padding: UiRect::vertical(Val::Px(style::properties::PADDING)),
            ..default()
        }
    }
}

#[derive(Component)]
pub(super) struct Contents;

impl Contents {
    fn bundle() -> impl Bundle {(
        Self,
        KeaObservers::<Self>::new(vec![
            Observer::new(on_contents_anim_complete),
        ]),
    )}
}
