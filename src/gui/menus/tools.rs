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
    app
        .add_systems(Update, on_add_menu_item_image)
        .add_observer(on_close_panel);
}

fn on_add_menu_item_image(
    images: Query<(Entity, &ChildOf), Added<ToolsMenuItemImage>>,
    items: Query<&ToolsMenuItem>,
    panels: Query<Entity, With<ToolsPanel>>,
    mut visibilities: Query<&mut Visibility>,
) {
    if images.is_empty() {
        return;
    }

    for (image, child) in &images {
        let Ok(item) = items.get(child.parent()) else {
            continue;
        };

        match item.item_type {
            ToolsMenuItemType::Tools => {
                let Ok(panel) = panels.single() else {
                    continue;
                };

                let Ok([mut visibility, panel_visibility]) = visibilities.get_many_mut([image, panel]) else {
                    continue;
                };

                *visibility = match *panel_visibility {
                    Visibility::Hidden => Visibility::Hidden,
                    _ => Visibility::Inherited,
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
    visibilities: Query<&Visibility>,
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
            let Ok(tools_panel) = tools_panels.single() else {
                return;
            };

            let Ok(visibility) = visibilities.get(tools_panel) else {
                return;
            };

            let new_vis = match visibility {
                Visibility::Hidden => Visibility::Visible,
                _ => Visibility::Hidden,
            };

            commands
                .entity(tools_panel)
                .insert(new_vis);
        },
    };

    commands.kea_popup_close();
}

fn on_close_panel(
    trigger: Trigger<KeaPanelClose>,
    tools_panels: Query<&ToolsPanel>,
    mut commands: Commands,
) {
    if !tools_panels.contains(trigger.target()) {
        return;
    }

    commands
        .entity(trigger.target())
        .insert(Visibility::Hidden);
}
