use bevy::prelude::*;
use bevy::ui::ContentSize;
use crate::gui::buttonex;
use crate::gui::sizer;
use crate::gui::style;
use super::events;
use super::Resources;

#[derive(Component)]
#[require(
    Node,
    BackgroundColor(|| style::colors::BACKGROUND),
    ContentSize,
)]
pub struct Panel;

impl Panel {
    pub(super) fn create(
        commands: &mut Commands,
        options: &events::Open,
        resources: &Res<Resources>,
    ) -> Entity {
        let mut entity = commands.spawn((
            Self,
            Node {
                flex_direction: FlexDirection::Column,
                left: Val::Px(options.position.x),
                top: Val::Px(options.position.y),
                width: Val::Px(options.size.x),
                height: Val::Px(options.size.y),
                min_width: Val::Px(20.0),
                min_height: Val::Px(20.0),
                padding: UiRect::all(Val::Px(style::properties::PADDING)),
                ..default()
            },
        ));

        let result = entity.id();

        entity.with_children(|parent| {
            parent
                .spawn(
                    Header {
                        panel: result,
                    },
                )
                .with_children(|parent| {
                    parent.spawn((
                        Title::new(&options.title),
                    ));

                    let node = Node {
                        align_self: AlignSelf::End,
                        ..default()
                    };

                    if let Some(icon) = resources.get_icon("icons/close.svg") {
                        buttonex::ButtonEx::create_image(parent, icon, Self::on_close, node);
                    } else {
                        buttonex::ButtonEx::create_label(parent, "x", Self::on_close, node);
                    }
                })
                .observe(Header::on_drag);

            parent.spawn(sizer::Sizer::new(sizer::Anchors::all()));
        });

        result
    }

    fn on_close(
        trigger: Trigger<buttonex::OnClick>,
        parents: Query<&Parent>,
        panels: Query<&Panel>,
        mut commands: Commands,
    ) {
        let root = parents.root_ancestor(trigger.entity());

        if !panels.contains(root) {
            return;
        }

        commands.entity(root).despawn_recursive();
    }
}

#[derive(Component)]
#[require(
    Node(Self::node),
    ContentSize
)]
struct Header {
    panel: Entity,
}

impl Header {
    fn on_drag(
        trigger: Trigger<Pointer<Drag>>,
        headers: Query<&Header>,
        mut panels: Query<&mut Node, With<Panel>>,
    ) {
        let Ok(header) = headers.get(trigger.target) else {
            return;
        };

        let Ok(mut panel) = panels.get_mut(header.panel) else {
            return;
        };

        let left = match panel.left {
            Val::Px(value) => value,
            _ => 0.0,
        };

        let top = match panel.top {
            Val::Px(value) => value,
            _ => 0.0,
        };

        let delta = trigger.delta;
        panel.left = Val::Px(left + delta.x);
        panel.top = Val::Px(top + delta.y);
    }

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Node(Self::node),
    Text,
    TextLayout(|| TextLayout::new_with_justify(JustifyText::Center)),
    TextFont(Self::text_font),
    PickingBehavior(|| PickingBehavior::IGNORE),
)]
struct Title;

impl Title {
    fn new(title: &str) -> impl Bundle {(
        Self,
        Text::new(title),
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            align_self: AlignSelf::Center,
            ..default()
        }
    }

    fn text_font() -> TextFont {
        TextFont {
            font_size: 12.0,
            ..default()
        }
    }
}
