use bevy::prelude::*;
use crate::settings::SettingsResource;
use kea::prelude::*;
use rfd::FileDialog;

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct LSP {
    language: String,
}

impl LSP {
    pub fn bundle() -> impl Bundle {(
        Self {
            language: String::new(),
        },
        children![
            Self::language_list(),
        ],
    )}

    fn language_list() -> impl Bundle {(
        Node {
            width: Val::Percent(100.0),
            row_gap: Val::Px(6.0),
            flex_direction: FlexDirection::Column,
            align_content: AlignContent::Stretch,
            ..default()
        },
        children![
            (
                KeaDropdown::bundle_custom(on_language_custom),
                KeaOnReadyComponent,
                KeaObservers::<Self>::new(vec![
                    Observer::new(on_ready),
                ]),
                KeaNodeOverrides {
                    width: Some(Val::Percent(100.0)),
                    ..default()
                },
            ),
            (
                Node {
                    width: Val::Percent(100.0),
                    column_gap: Val::Px(6.0),
                    ..default()
                },
                children![
                    (
                        KeaPropertyText::bundle("Path", ""),
                    ),
                    (
                        KeaButton::image_bundle(
                            &format!("kea://icons/search.svg#image{size}x{size}",
                                size = kea::style::properties::FONT_SIZE,
                            ),
                            on_select_directory,
                        ),
                    ),
                ],
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            display: Display::None,
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(kea::style::properties::ROW_GAP),
            ..default()
        }
    }
}

#[derive(Component)]
struct LanguageList {
    dropdown: Entity,
}

fn on_language_custom(
    trigger: Trigger<KeaDropdownCustom>,
    settings: Res<SettingsResource>,
    mut commands: Commands,
) {
    let keys: Vec<String> = settings.app.lsp.servers
        .keys()
        .map(|element| element.clone())
        .collect();

    let event = trigger.event();
    let entity = commands.spawn((
        Node {
            width: Val::Percent(100.0),
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(6.0),
            ..default()
        },
        BackgroundColor(kea::style::colors::BACKGROUND),
        children![
            (
                KeaList::new(KeaListBehavior::Select),
                KeaListLabelItems(keys),
                KeaObservers::<LanguageList>::new(vec![
                    Observer::new(on_language_custom_select),
                ]),
                LanguageList {
                    dropdown: trigger.target(),
                },
            ),
            (
                Node {
                    width: Val::Percent(100.0),
                    column_gap: Val::Px(6.0),
                    align_content: AlignContent::Stretch,
                    ..default()
                },
                children![
                    (
                        KeaTextInput::bundle(),
                        KeaNodeOverrides {
                            flex_grow: Some(1.0),
                            ..default()
                        }
                    ),
                    (
                        KeaButton::image_bundle(
                            &format!("kea://icons/plus.svg#image{size}x{size}",
                                size = kea::style::properties::FONT_SIZE,
                            ),
                            on_language_custom_add,
                        ),
                    ),
                ]
            ),
        ],
    ))
    .id();

    commands
        .entity(event.entity)
        .add_child(entity);
}

fn on_language_custom_select(
    trigger: Trigger<KeaListSelect>,
    texts: Query<&Text>,
    lists: Query<&LanguageList>,
    parents: Query<&Children>,
    children: Query<&ChildOf>,
    properties: Query<Entity, With<KeaPropertyText>>,
    settings: Res<SettingsResource>,
    mut lsps: Query<&mut LSP>,
    mut commands: Commands,
) {
    let event = trigger.event();

    let Ok(text) = texts.get(event.entity) else {
        panic!("Failed to get the Text component for the selected list item.");
    };

    let language = text.0.clone();
    let server = match settings.app.lsp.servers.get(&language) {
        Some(value) => value.clone(),
        None => format!(""),
    };

    let Ok(list) = lists.get(trigger.target()) else {
        panic!("Could not get LanguageList component for custom dropdown.");
    };

    update_language_server(
        list.dropdown,
        server,
        &parents,
        &children,
        &properties,
        &mut commands
    );

    for parent in children.iter_ancestors(list.dropdown) {
        let Ok(mut lsp) = lsps.get_mut(parent) else {
            continue;
        };

        lsp.language = language.clone();
        break;
    }

    commands
        .kea_dropdown_set_label(list.dropdown, language)
        .kea_popup_close();
}

fn on_language_custom_add(
    trigger: Trigger<KeaButtonClick>,
    children: Query<&ChildOf>,
    parents: Query<&Children>,
    inputs: Query<&KeaTextInput>,
    texts: Query<&Text>,
    lsp_panels: Query<Entity, With<LSP>>,
    dropdowns: Query<&KeaDropdown>,
    properties: Query<Entity, With<KeaPropertyText>>,
    mut settings: ResMut<SettingsResource>,
    mut commands: Commands,
) {
    commands.kea_popup_close();

    let Ok(child) = children.get(trigger.target()) else {
        panic!("Failed to get ChildOf component for 'Add' button.");
    };

    for descendant in parents.iter_descendants(child.parent()) {
        let Ok(input) = inputs.get(descendant) else {
            continue;
        };

        let Ok(text) = texts.get(input.contents_entity()) else {
            continue;
        };

        let server = text.0.clone();

        for lsp_panel in lsp_panels {
            for lsp_panel_child in parents.iter_descendants(lsp_panel) {
                if !dropdowns.contains(lsp_panel_child) {
                    continue;
                }

                commands.kea_dropdown_set_label(lsp_panel_child, server.clone());

                update_language_server(
                    lsp_panel_child,
                    format!(""),
                    &parents,
                    &children,
                    &properties,
                    &mut commands,
                );
                break;
            }
        }

        settings.app.lsp.servers.insert(server, format!(""));
        break;
    }
}

fn update_language_server(
    target: Entity,
    server: String,
    parents: &Query<&Children>,
    children: &Query<&ChildOf>,
    properties: &Query<Entity, With<KeaPropertyText>>,
    commands: &mut Commands,
) {
    let Ok(child) = children.get(target) else {
        return;
    };

    for descendant in parents.iter_descendants(child.parent()) {
        let Ok(property) = properties.get(descendant) else {
            continue;
        };

        commands.kea_property_set_value(property, server);
        break;
    }
}

fn on_ready(
    trigger: Trigger<KeaOnReady>,
    parents: Query<&Children>,
    children: Query<&ChildOf>,
    properties: Query<Entity, With<KeaPropertyText>>,
    settings: Res<SettingsResource>,
    mut commands: Commands,
) {
    let keys: Vec<String> = settings.app.lsp.servers
        .keys()
        .map(|element| element.clone())
        .collect();

    if keys.is_empty() {
        return;
    }

    let server = match settings.app.lsp.servers.get(&keys[0]) {
        Some(value) => value.clone(),
        None => format!(""),
    };

    commands.kea_dropdown_set_label(trigger.target(), keys[0].clone());

    update_language_server(
        trigger.target(),
        server,
        &parents,
        &children,
        &properties,
        &mut commands
    );
}

fn on_select_directory(
    trigger: Trigger<KeaButtonClick>,
    parents: Query<&Children>,
    children: Query<&ChildOf>,
    properties: Query<Entity, With<KeaPropertyText>>,
    lsps: Query<&LSP>,
    mut settings: ResMut<SettingsResource>,
    mut commands: Commands,
) {
    let Some(response) = FileDialog::new().pick_file() else {
        return;
    };

    let Some(server) = response.to_str() else {
        return;
    };

    let mut language = String::new();
    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(lsp) = lsps.get(parent) else {
            continue;
        };

        language = lsp.language.clone();
        break;
    }

    if settings.app.lsp.servers.contains_key(&language) {
        settings.app.lsp.servers.insert(language, server.to_string());
    }

    let Ok(child) = children.get(trigger.target()) else {
        return;
    };

    update_language_server(
        child.parent(),
        server.to_string(),
        &parents,
        &children,
        &properties,
        &mut commands
    );
}
