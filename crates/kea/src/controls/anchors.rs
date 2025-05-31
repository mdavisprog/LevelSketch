use bevy::prelude::*;
use bitflags;
use crate::style;

bitflags::bitflags! {
    ///
    /// KeaAnchors
    ///
    /// Bit flags to determine which anchors of a parent node is resizable.
    ///
    #[derive(Clone, Copy, Debug)]
    pub struct KeaAnchors: u16 {
        const LEFT = 1 << 0;
        const TOP = 1 << 1;
        const RIGHT = 1 << 2;
        const BOTTOM = 1 << 3;
        const TOPLEFT = 1 << 4;
        const TOPRIGHT = 1 << 5;
        const BOTTOMLEFT = 1 << 6;
        const BOTTOMRIGHT = 1 << 7;
    }
}

impl KeaAnchors {
    pub fn bounds_from_position(size: Vec2, position: Vec2) -> (Rect, Self) {
        let anchors = Self::to_array();

        for anchor in anchors {
            if anchor.bounds(size).contains(position) {
                return (anchor.bounds(size), anchor);
            }
        }

        (Rect::EMPTY, Self::empty())
    }

    fn bounds(&self, size: Vec2) -> Rect {
        let padding = style::properties::SIZER_SIZE;

        if self.contains(Self::LEFT) {
            Rect::new(
                0.0,
                0.0,
                padding,
                size.y,
            )
        } else if self.contains(Self::TOP) {
            Rect::new(
                0.0,
                0.0,
                size.x,
                padding,
            )
        } else if self.contains(Self::RIGHT) {
            Rect::new(
                size.x - padding,
                0.0,
                size.x,
                size.y,
            )
        } else if self.contains(Self::BOTTOM) {
            Rect::new(
                0.0,
                size.y - padding,
                size.x,
                size.y,
            )
        } else if self.contains(Self::TOPLEFT) {
            Rect::new(
                0.0,
                0.0,
                padding * 2.0,
                padding * 2.0,
            )
        } else if self.contains(Self::TOPRIGHT) {
            Rect::new(
                size.x - padding * 2.0,
                0.0,
                size.x,
                padding * 2.0,
            )
        } else if self.contains(Self::BOTTOMRIGHT) {
            Rect::new(
                size.x - padding * 2.0,
                size.y - padding * 2.0,
                size.x,
                size.y,
            )
        } else if self.contains(Self::BOTTOMLEFT) {
            Rect::new(
                0.0,
                size.y - padding * 2.0,
                padding * 2.0,
                size.y,
            )
        } else {
            Rect::EMPTY
        }
    }

    fn to_array() -> [Self; 8] {[
        Self::TOPLEFT,
        Self::TOPRIGHT,
        Self::BOTTOMRIGHT,
        Self::BOTTOMLEFT,
        Self::LEFT,
        Self::TOP,
        Self::RIGHT,
        Self::BOTTOM,
    ]}
}
