use bevy::{
    ecs::system::IntoObserverSystem,
    prelude::*,
};
use crate::{
    constants,
    controls::image::KeaImageNode,
    observers::KeaObservers,
    style,
};
use super::systems;

///
/// KeaButton
///
#[derive(Component)]
#[require(
    Node = Self::node(),
    BackgroundColor = style::colors::BACKGROUND,
    ZIndex(constants::BASE_Z_INDEX),
)]
pub struct KeaButton {
    _private: (),
}

impl KeaButton {
    pub fn label_bundle<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>,
        label: &str,
    ) -> impl Bundle {(
        Self::bundle(callback),
        children![(
            Text::new(label),
            TextFont::from_font_size(12.0),
            TextLayout::new_with_justify(JustifyText::Center),
            Pickable::IGNORE,
        )],
    )}

    pub fn image_bundle<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>,
        path: &str,
    ) -> impl Bundle {(
        Self::bundle(callback),
        children![(
            KeaImageNode(path.to_string()),
            Pickable::IGNORE,
        )],
    )}

    fn bundle<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>,
    ) -> impl Bundle {(
        Self {
            _private: (),
        },
        KeaObservers::new(vec![
            Observer::new(callback),
            Observer::new(systems::mouse_over),
            Observer::new(systems::mouse_out),
            Observer::new(systems::mouse_pressed),
            Observer::new(systems::mouse_click),
        ]),
    )}

    fn node() -> Node {
        Node {
            justify_content: JustifyContent::Center,
            padding: style::properties::BUTTON_PADDING,
            ..default()
        }
    }
}
