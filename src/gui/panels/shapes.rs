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
                parent
                    .spawn(Item)
                    .with_children(|parent| {
                        parent.spawn(Inner);
                    })
                    .observe(Item::on_over)
                    .observe(Item::on_out);
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
    fn on_over(
        trigger: Trigger<Pointer<Over>>,
        mut items: Query<&mut BackgroundColor, With<Item>>,
    ) {
        let Ok(mut item_color) = items.get_mut(trigger.target) else {
            return;
        };

        *item_color = style::colors::HIGHLIGHT.into();
    }

    fn on_out(
        trigger: Trigger<Pointer<Out>>,
        mut items: Query<&mut BackgroundColor, With<Item>>,
    ) {
        let Ok(mut item_color) = items.get_mut(trigger.target) else {
            return;
        };

        *item_color = style::colors::NORMAL.into();
    }

    fn node() -> Node {
        Node {
            width: Val::Px(60.0),
            height: Val::Px(60.0),
            align_items: AlignItems::Center,
            justify_content: JustifyContent::Center,
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Node(Self::node),
    BackgroundColor(|| Color::srgb(0.3, 0.6, 0.3)),
    PickingBehavior(|| PickingBehavior::IGNORE),
)]
struct Inner;

impl Inner {
    fn node() -> Node {
        Node {
            width: Val::Px(40.0),
            height: Val::Px(40.0),
            ..default()
        }
    }
}
