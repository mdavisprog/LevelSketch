use bevy::prelude::*;
use crate::{
    constants,
    observers::KeaObservers,
    style,
};
use super::systems::{
    scrollbar,
};

/// Parent component that holds each scrollbar. Placed in a separate node so that scrollbar sizes
/// don't influence the main contents of the scrollable area.
#[derive(Component)]
#[require(
    Node = Self::node(),
    ZIndex(constants::SIZER_Z_INDEX),
    Pickable::IGNORE,
)]
pub(super) struct ScrollbarsContainer {
    _private: (),
}

impl ScrollbarsContainer {
    pub fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            (
                Scrollbar::vertical(),
            ),
            (
                Scrollbar::horizontal(),
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            position_type: PositionType::Absolute,
            ..default()
        }
    }
}

#[derive(Clone, Copy, Debug)]
pub(super) enum ScrollbarType {
    Horizontal,
    Vertical,
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub(super) enum ScrollbarHoverState {
    Collapsed,
    Expanded,
}

#[derive(Component)]
#[require(
    BackgroundColor(style::colors::BUTTON_BACKGROUND),
    ZIndex(constants::SIZER_Z_INDEX),
    KeaObservers<Self> = Self::observers(),
)]
pub(super) struct Scrollbar {
    pub hover_state: ScrollbarHoverState,
    bar_type: ScrollbarType,
}

impl Scrollbar {
    // This is the offset from the edge of the scrollbar to determine if the size should be
    // expanded/collapsed.
    const HOVER_SIZE: f32 = 20.0;

    pub fn vertical() -> impl Bundle {(
        Self {
            bar_type: ScrollbarType::Vertical,
            hover_state: ScrollbarHoverState::Collapsed,
        },
        Node {
            position_type: PositionType::Absolute,
            ..default()
        }
    )}

    pub fn horizontal() -> impl Bundle {(
        Self {
            bar_type: ScrollbarType::Horizontal,
            hover_state: ScrollbarHoverState::Collapsed,
        },
        Node {
            position_type: PositionType::Absolute,
            ..default()
        }
    )}

    pub fn bar_type(&self) -> ScrollbarType {
        self.bar_type
    }

    pub fn hover_bounds(&self, bounds: Rect) -> Rect {
        match self.bar_type {
            ScrollbarType::Horizontal => {
                Rect::new(
                    bounds.min.x,
                    bounds.max.y - Self::HOVER_SIZE,
                    bounds.max.x,
                    bounds.max.y,
                )
            },
            ScrollbarType::Vertical => {
                Rect::new(
                    bounds.max.x - Self::HOVER_SIZE,
                    bounds.min.y,
                    bounds.max.x,
                    bounds.max.y,
                )
            },
        }
    }

    fn observers() -> KeaObservers<Self> {
        KeaObservers::new(vec![
            Observer::new(scrollbar::on_over),
            Observer::new(scrollbar::on_out),
            Observer::new(scrollbar::on_drag),
            Observer::new(scrollbar::on_pressed),
        ])
    }
}
