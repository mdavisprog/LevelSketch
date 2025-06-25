use bevy::{
    ecs::system::IntoObserverSystem,
    prelude::*,
};
use crate::{
    observers::KeaObservers,
    style,
};

#[derive(Component)]
#[require(
    Node = Self::node(),
    BorderRadius = style::properties::BORDER_RADIUS,
    BackgroundColor = style::colors::BUTTON_BACKGROUND,
)]
pub struct KeaTextInput {
    _private: (),
}

impl KeaTextInput {
    pub fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            TextInput,
        ]
    )}

    pub fn bundle_with_callback<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>
    ) -> impl Bundle {(
        Self::bundle(),
        KeaObservers::new(vec![
            Observer::new(callback),
        ]),
    )}

    fn node() -> Node {
        Node {
            overflow: Overflow::clip(),
            width: Val::Px(75.0),
            height: Val::Px(style::properties::CONTROL_HEIGHT),
            padding: UiRect::horizontal(Val::Px(6.0)),
            justify_content: JustifyContent::Center,
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Node = Self::node(),
    Text,
    TextFont = TextFont::from_font_size(style::properties::FONT_SIZE),
    TextLayout = TextLayout::new_with_linebreak(LineBreak::NoWrap),
    Pickable = Pickable::IGNORE,
)]
pub(super) struct TextInput;

impl TextInput {
    fn node() -> Node {
        Node {
            align_self: AlignSelf::Center,
            ..default()
        }
    }
}
