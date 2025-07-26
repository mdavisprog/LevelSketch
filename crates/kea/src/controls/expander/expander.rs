use bevy::{
    ecs::{
        component::HookContext,
        world::DeferredWorld,
    },
    prelude::*,
};
use crate::{
    controls::{
        image::KeaImageNode,
        separator::KeaSeparator,
    },
    style,
};
use super::systems::{
    on_click_header,
    on_contents_anim_complete,
    on_expander_event,
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
)]
pub struct KeaExpander {
    pub(super) state: KeaExpanderState,
    pub(super) expanded_height: f32,
}

impl KeaExpander {
    pub fn bundle(header: impl Bundle, contents: impl Bundle) -> impl Bundle {(
        Self::new(),
        children![
            (
                header,
                Listener,
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
                Listener,
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
            Contents,
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
}

#[derive(Component)]
#[component(
    on_add = Self::on_add,
)]
struct Listener;

impl Listener {
    fn on_add(
        mut world: DeferredWorld,
        HookContext {
            entity,
            ..
        }: HookContext,
    ) {
        let mut commands = world.commands();
        commands
            .entity(entity)
            .remove::<Self>()
            .observe(on_click_header);
    }
}

#[derive(Component)]
#[component(
    on_add = Self::on_add,
)]
#[require(
    Node = Self::node(),
)]
pub(super) struct Header;

impl Header {
    fn bundle(label: &str) -> impl Bundle {(
        Self,
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

    fn on_add(
        mut world: DeferredWorld,
        HookContext {
            entity,
            ..
        }: HookContext,
    ) {
        let mut commands = world.commands();
        commands
            .entity(entity)
            .observe(on_expander_event);
    }
}

#[derive(Component)]
#[component(
    on_add = Self::on_add,
)]
pub(super) struct Contents;

impl Contents {
    fn on_add(
        mut world: DeferredWorld,
        HookContext {
            entity,
            ..
        }: HookContext,
    ) {
        let mut commands = world.commands();
        commands
            .entity(entity)
            .observe(on_contents_anim_complete);
    }
}
