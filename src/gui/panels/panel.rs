use bevy::prelude::*;
use bevy::ui::ContentSize;
use crate::gui::buttonex;
use crate::gui::sizer;
use crate::gui::style;
use super::Resources;

pub struct PanelOptions {
    pub title: String,
    pub position: Vec2,
    pub size: Vec2,
}

impl Default for PanelOptions {
    fn default() -> Self {
        Self {
            title: "Panel".into(),
            position: Vec2::ZERO,
            size: Vec2::new(100.0, 200.0),
        }
    }
}

pub struct PanelResult {
    pub panel: Entity,
    // The entity that contains the caller's components.
    pub components: Entity,
}

#[derive(Component)]
#[require(
    Node,
    BackgroundColor = style::colors::BACKGROUND,
    ContentSize,
)]
pub struct Panel;

impl Panel {
    pub fn create(
        commands: &mut Commands,
        options: &PanelOptions,
        resources: &Res<Resources>,
        components: impl Bundle,
    ) -> PanelResult {
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

        let mut result = PanelResult {
            panel: entity.id(),
            components: Entity::PLACEHOLDER,
        };

        entity.with_children(|parent| {
            parent
                .spawn(
                    Header {
                        panel: result.panel,
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

            result.components = parent.spawn(components).id();
            parent.spawn(sizer::Sizer::new(sizer::Anchors::all()));
        });

        result
    }

    fn on_close(
        trigger: Trigger<buttonex::OnClick>,
        child_of: Query<&ChildOf>,
        panels: Query<&Panel>,
        mut commands: Commands,
    ) {
        let root = child_of.root_ancestor(trigger.target());

        if !panels.contains(root) {
            return;
        }

        commands.entity(root).despawn();
    }
}

#[derive(Component)]
#[require(
    Node = Self::node(),
    ContentSize,
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
    Node = Self::node(),
    Text,
    TextLayout = TextLayout::new_with_justify(JustifyText::Center),
    TextFont = Self::text_font(),
    Pickable = Pickable::IGNORE,
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
