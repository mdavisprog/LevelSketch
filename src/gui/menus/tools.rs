use bevy::{
    platform::collections::HashMap,
    prelude::*,
};
use crate::gui::tools::{
    ToolsPanel,
    ToolsPanelType,
};
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
                    Observer::new(on_ready),
                    Observer::new(on_select),
                ]),
                KeaOnReadyComponent,
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

#[derive(Component)]
struct ToolsMenuItemImage;

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
struct ToolsMenuItem;

impl ToolsMenuItem {
    fn bundle(label: &str, is_checked: bool) -> impl Bundle {(
        Self,
        children![
            (
                KeaImageNode(format!("kea://icons/check.svg#image18x18")),
                ToolsMenuItemImage,
                if is_checked { Visibility::Inherited } else { Visibility::Hidden },
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
    app.add_observer(on_close_panel);
}

fn on_ready(
    trigger: Trigger<KeaOnReady>,
    panels: Query<(&ToolsPanelType, &Visibility)>,
    mut commands: Commands,
) {
    // Controls the ordering of how the menu items will appear.
    const ITEMS: [ToolsPanelType; 3] = [
        ToolsPanelType::Project,
        ToolsPanelType::Assets,
        ToolsPanelType::Properties,
    ];

    let mut visibilities = HashMap::<ToolsPanelType, bool>::new();

    for (panel, visibility) in panels {
        let is_checked = match visibility {
            Visibility::Hidden => false,
            _ => true,
        };

        visibilities.insert(*panel, is_checked);
    }

    for item in ITEMS {
        commands
            .entity(trigger.target())
            .with_child((
                ToolsMenuItem::bundle(&format!("{item:?}"), visibilities[&item]),
                item,
            ));
    }
}

fn on_select(
    trigger: Trigger<KeaListSelect>,
    parents: Query<&Children>,
    visibilities: Query<&Visibility>,
    panels: Query<Entity, With<ToolsPanel>>,
    panel_types: Query<&ToolsPanelType>,
    mut commands: Commands,
) {
    let Ok(list) = parents.get(trigger.target()) else {
        return;
    };

    let event = trigger.event();
    let Some(child) = list.get(event.index) else {
        return;
    };

    let Ok(item_type) = panel_types.get(*child) else {
        return;
    };

    for panel in panels {
        let Ok(panel_type) = panel_types.get(panel) else {
            continue;
        };

        if item_type != panel_type {
            continue;
        }

        let Ok(visibility) = visibilities.get(panel) else {
            continue;
        };

        commands
            .entity(panel)
            .insert(match visibility {
                Visibility::Hidden => Visibility::Visible,
                _ => Visibility::Hidden,
            });
        commands.kea_panel_focus(panel);

        break;
    }

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
