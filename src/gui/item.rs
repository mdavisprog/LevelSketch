use bevy::prelude::*;
use kea::prelude::*;

/// Represents a hoverable item with contents on the inside such as an image and other text
/// labels to describe what this item is.
#[derive(Component)]
#[require(
    Node = Self::node(),
    BackgroundColor(kea::style::colors::PRESSED),
)]
pub struct Item;

impl Item {
    pub fn bundle(
        contents: impl Bundle,
    ) -> impl Bundle {(
        Self,
        children![
            (
                contents,
            ),
            (
                KeaObservers::new_observe_parent(vec![
                    Observer::new(on_over),
                    Observer::new(on_out),
                ]),
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            align_items: AlignItems::Center,
            justify_content: JustifyContent::Center,
            padding: UiRect::all(Val::Px(10.0)),
            ..default()
        }
    }
}

fn on_over(
    trigger: Trigger<Pointer<Over>>,
    mut commands: Commands,
) {
    commands
        .entity(trigger.target())
        .insert(BackgroundColor(kea::style::colors::HIGHLIGHT));
}

fn on_out(
    trigger: Trigger<Pointer<Out>>,
    mut commands: Commands,
) {
    commands
        .entity(trigger.target())
        .insert(BackgroundColor(kea::style::colors::PRESSED));
}
