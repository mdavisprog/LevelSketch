use bevy::prelude::*;
use bevy::ui::ContentSize;
use crate::svg;
use super::buttonex;
use super::icons;
use super::sizer;
use super::style;

pub mod events {
    use super::*;

    #[derive(Event)]
    pub struct Open {
        pub position: Vec2,
        pub title: String,
    }
}

#[derive(Component)]
#[require(
    Node,
    BackgroundColor(|| style::colors::BACKGROUND),
    ContentSize,
)]
pub struct Panel;

impl Panel {
    pub fn initialize(app: &mut App) {
        app
            .add_event::<events::Open>()
            .add_systems(Update, Self::on_open);
    }

    fn create(
        commands: &mut Commands,
        position: Vec2,
        title: &str,
        asset_server: &Res<AssetServer>,
        svgs: &Res<Assets<svg::SvgAsset>>,
        icons: &mut ResMut<icons::Icons>,
    ) -> Entity {
        let mut entity = commands.spawn((
            Self,
            Node {
                flex_direction: FlexDirection::Column,
                left: Val::Px(position.x),
                top: Val::Px(position.y),
                width: Val::Px(100.0),
                height: Val::Px(200.0),
                padding: UiRect::all(Val::Px(style::properties::PADDING)),
                ..default()
            },
        ));

        let result = entity.id();

        entity.with_children(|parent| {
            parent
                .spawn((
                    Header {
                        panel: result,
                    },
                    Node {
                        width: Val::Percent(100.0),
                        ..default()
                    },
                ))
                .with_children(|parent| {
                    parent.spawn((
                        Text::new(title),
                        TextFont {
                            font_size: 12.0,
                            ..default()
                        },
                        TextLayout::new_with_justify(JustifyText::Center),
                        Node {
                            width: Val::Percent(100.0),
                            align_self: AlignSelf::Center,
                            ..default()
                        },
                        PickingBehavior::IGNORE,
                    ));

                    let node = Node {
                        align_self: AlignSelf::End,
                        ..default()
                    };

                    if let Ok(icon) = icons.get_size("icons/close.svg", Vec2::new(12.0, 12.0), &asset_server, &svgs) {
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

    fn on_open(
        asset_server: Res<AssetServer>,
        svgs: Res<Assets<svg::SvgAsset>>,
        mut events: EventReader<events::Open>,
        mut commands: Commands,
        mut icons: ResMut<icons::Icons>,
    ) {
        for event in events.read() {
            Self::create(
                &mut commands,
                event.position,
                &event.title,
                &asset_server,
                &svgs,
                &mut icons
            );
        }
    }
}

#[derive(Component)]
#[require(Node, ContentSize)]
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
}
