use bevy::prelude::*;
use crate::gui::tools::tools::Tools;
use kea::prelude::*;
use super::lsp::LSP;

#[derive(Component)]
#[require(Tools)]
pub struct SettingsTools {
    _private: (),
}

impl SettingsTools {
    pub fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        KeaNodeOverrides {
            height: Some(Val::Percent(100.0)),
            ..default()
        },
        children![
            Self::components(),
        ],
    )}

    fn components() -> impl Bundle {(
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            column_gap: Val::Px(kea::style::properties::ROW_GAP),
            ..default()
        },
        children![
            (
                KeaList::new_with_selected(0),
                KeaNodeOverrides {
                    width: Some(Val::Auto),
                    height: Some(Val::Percent(100.0)),
                    ..default()
                },
                KeaObservers::<SettingsType>::new(vec![
                    Observer::new(on_settings_selected),
                ]),
                children![
                    Self::list_item("App"),
                    Self::list_item("LSP"),
                ],
            ),
            (
                KeaSeparator::vertical(),
            ),
            (
                Node {
                    width: Val::Percent(100.0),
                    ..default()
                },
                children![
                    Self::app(),
                    Self::lsp(),
                ],
            ),
        ],
    )}

    fn app() -> impl Bundle {(
        SettingsType::App,
        Node::default(),
        children![
            (
                KeaLabel::bundle("App"),
            ),
        ],
    )}

    fn lsp() -> impl Bundle {(
        SettingsType::LSP,
        LSP::bundle(),
    )}

    fn list_item(label: &str) -> impl Bundle {(
        Node {
            padding: UiRect::axes(Val::Px(12.0), Val::Px(6.0)),
            align_items: AlignItems::Center,
            justify_items: JustifyItems::Center,
            ..default()
        },
        children![
            (
                KeaLabel::bundle_with_size(label, 18.0),
            ),
        ],
    )}
}

#[derive(Component, PartialEq, Eq, Copy, Clone)]
enum SettingsType {
    App,
    LSP,
}

fn on_settings_selected(
    trigger: Trigger<KeaListSelect>,
    mut settings_types: Query<(&SettingsType, &mut Node)>,
) {
    let event = trigger.event();

    for (settings, mut node) in &mut settings_types {
        node.display = if *settings as usize == event.index {
            Display::Flex
        } else {
            Display::None
        };
    }
}
