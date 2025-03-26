use bevy::prelude::*;
use crate::gui::style;
use super::*;

#[derive(Component)]
#[require(
    Node(Self::node)
)]
pub struct Shapes;

impl Shapes {
    pub fn create(
        commands: &mut Commands,
        resources: &Res<Resources>,
        position: Vec2,
    ) {
        let options = PanelOptions {
            title: format!("Shapes"),
            position,
            size: Vec2::new(400.0, 200.0),
            ..default()
        };

        let result = Panel::create(commands, &options, resources, Self);
        commands
            .entity(result.components)
            .with_children(|parent| {
                parent.spawn(Item);
            });
    }

    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Row,
            flex_wrap: FlexWrap::Wrap,
            row_gap: Val::Px(8.0),
            column_gap: Val::Px(8.0),
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Node(Self::node),
    BackgroundColor(|| style::colors::NORMAL),
)]
struct Item;

impl Item {
    fn node() -> Node {
        Node {
            width: Val::Px(40.0),
            height: Val::Px(40.0),
            ..default()
        }
    }
}
