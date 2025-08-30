use bevy::prelude::*;
use crate::gui::tools::ToolsPanel;
use kea::prelude::*;

#[derive(Component)]
#[require(
    Node = Self::node(),
    BackgroundColor(kea::style::colors::BACKGROUND),
)]
pub struct ToolsMenu {
    _private: (),
}

impl ToolsMenu {
    pub fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            (
                KeaList::new(KeaListBehavior::NoSelect),
                KeaObservers::<Self>::new(vec![
                    Observer::new(on_select),
                ]),
                children![
                    ToolsMenuItem::bundle("Tools", ToolsMenuItemType::Tools),
                ],
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            ..default()
        }
    }
}

#[derive(Clone, Copy)]
enum ToolsMenuItemType {
    Tools,
}

#[derive(Component)]
struct ToolsMenuItemImage;

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
struct ToolsMenuItem {
    item_type: ToolsMenuItemType,
}

impl ToolsMenuItem {
    fn bundle(label: &str, item_type: ToolsMenuItemType) -> impl Bundle {(
        Self {
            item_type,
        },
        children![
            (
                KeaImageNode(format!("kea://icons/check.svg#image18x18")),
                ToolsMenuItemImage,
                Visibility::Hidden,
            ),
            (
                KeaLabel::bundle(label),
            ),
        ]
    )}

    fn node() -> Node {
        Node {
            align_items: AlignItems::Center,
            column_gap: Val::Px(12.0),
            padding: UiRect::vertical(Val::Px(4.0)),
            ..default()
        }
    }
}

pub(super) fn build(app: &mut App) {
    app.add_systems(Update, on_add_menu_item_image);
}

fn on_add_menu_item_image(
    items: Query<&ToolsMenuItem>,
    panels: Query<&ToolsPanel>,
    mut images: Query<(&ChildOf, &mut Visibility), Added<ToolsMenuItemImage>>,
) {
    if images.is_empty() {
        return;
    }

    for (image, mut visibility) in &mut images {
        let Ok(item) = items.get(image.parent()) else {
            continue;
        };

        match item.item_type {
            ToolsMenuItemType::Tools => {
                *visibility = if panels.is_empty() {
                    Visibility::Hidden
                } else {
                    Visibility::Inherited
                };
            },
        }
    }
}

fn on_select(
    trigger: Trigger<KeaListSelect>,
    parents: Query<&Children>,
    tools_items: Query<&ToolsMenuItem>,
    tools_panels: Query<Entity, With<ToolsPanel>>,
    mut commands: Commands,
) {
    let Ok(list) = parents.get(trigger.target()) else {
        return;
    };

    let event = trigger.event();
    let Some(child) = list.get(event.index) else {
        return;
    };

    let Ok(tools_item) = tools_items.get(*child) else {
        return;
    };

    match tools_item.item_type {
        ToolsMenuItemType::Tools => {
            if tools_panels.is_empty() {
                commands.spawn(ToolsPanel::bundle());
            } else {
                for panel in tools_panels {
                    commands
                        .entity(panel)
                        .despawn();
                }
            }
        },
    }

    commands.kea_popup_close();
}
