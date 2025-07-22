use bevy::{
    color,
    ecs::{
        query::QueryData,
        system::{
            IntoObserverSystem,
            SystemId,
            SystemState,
        },
    },
    input::mouse::AccumulatedMouseMotion,
    prelude::*, 
    render::camera::RenderTarget,
    window::{
        PrimaryWindow,
        WindowRef,
    }
};
use crate::{
    controls::{
        button::*,
        list::*,
        panel::{
            KeaPanel,
            KeaPanelOptions,
        },
        scroll::KeaScrollable,
    },
    observers::KeaObservers,
    overrides::KeaNodeOverrides,
    utility,
};

///
/// KeaInspector
///
/// Tools panel that allows the inspection of all Node entities under a mouse
/// position. This allows the user to drill down to view the entities and 
///
#[derive(Component)]
pub struct KeaInspector {
    /// This inspector's list of hovered entities.
    hovered: Vec<Entity>,

    /// The currently selected entity in the hovered list.
    selected: Entity,

    // The current state of this inspector.
    state: State,
}

impl KeaInspector {
    pub fn bundle() -> impl Bundle {
        KeaPanel::bundle(KeaPanelOptions {
            title: format!("Inspector"),
            position: Vec2::new(100.0, 100.0),
            size: Vec2::new(400.0, 500.0),
        },
        (
            Self {
                hovered: Vec::new(),
                selected: Entity::PLACEHOLDER,
                state: State::None,
            },
            Node {
                flex_direction: FlexDirection::Column,
                width: Val::Percent(100.0),
                ..default()
            },
            children![
                // Buttons for selecting/clearing the hovered entities.
                (
                    Node {
                        flex_direction: FlexDirection::Row,
                        align_content: AlignContent::Stretch,
                        column_gap: Val::Px(4.0),
                        width: Val::Percent(100.0),
                        ..default()
                    },
                    children![
                        (
                            Self::button(on_select_hovered, "Select", ContentType::SelectButton),
                        ),
                        (
                            Self::button(on_clear_hovered, "Clear", ()),
                        ),
                    ],
                ),
                // Information about the entities.
                (
                    Contents::bundle(),
                ),
            ]
        ))
    }

    fn button<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>,
        label: &str,
        bundles: impl Bundle,
    ) -> impl Bundle {(
        KeaButton::label_bundle(label, callback),
        KeaNodeOverrides {
            flex_grow: Some(1.0),
            ..default()
        },
        bundles,
    )}
}

pub(super) struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        let resource = Data::new(
            app.register_system(update_component_info),
        );

        app
            .insert_resource(resource)
            .add_systems(Update, (update_hovered, update_inspectors).chain())
            .add_observer(on_add)
            .add_observer(on_remove);
    }
}

///
/// Contents
///
/// This node contains the list of hovered entities and allows
/// inspecting the components of a selected entity.
///
#[derive(Component)]
#[require(
    Node = Self::node(),
)]
struct Contents;

impl Contents {
    fn bundle() -> impl Bundle {(
        Self,
        children![
            (
                Node {
                    flex_grow: 1.0,
                    overflow: Overflow::scroll(),
                    width: Val::Percent(100.0),
                    ..default()
                },
                KeaScrollable,
                children![
                    (
                        KeaNodeOverrides {
                            display: Some(Display::None),
                            ..default()
                        },
                        KeaList::default(),
                        ContentType::HoverList,
                        KeaObservers::new(vec![
                            Observer::new(on_select_list_item),
                            Observer::new(on_hover_list_item),
                        ]),
                    ),
                    (
                        Text::new(""),
                        TextFont::from_font_size(12.0),
                        ContentType::HoverInfo,
                    ),
                ],
            ),
            (
                Node {
                    flex_grow: 1.0,
                    width: Val::Percent(100.0),
                    overflow: Overflow::scroll(),
                    ..default()
                },
                Text::new(""),
                TextFont::from_font_size(12.0),
                ContentType::ComponentInfo,
            )
        ],
    )}

    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Row,
            align_content: AlignContent::Stretch,
            ..default()
        }
    }
}

///
/// NodeOutline
///
/// Node that draws a highlighted rectangle around the hovered node.
///
#[derive(Component)]
#[require(
    Node = Self::node(),
    Outline = Self::outline(),
    GlobalZIndex(i32::MAX),
    Pickable = Pickable::IGNORE,
    Visibility = Visibility::Hidden,
)]
struct NodeOutline {
    inspector: Entity,
}

impl NodeOutline {
    fn node() -> Node {
        Node {
            position_type: PositionType::Absolute,
            width: Val::Px(25.0),
            height: Val::Px(25.0),
            ..default()
        }
    }

    fn outline() -> Outline {
        Outline { 
            width: Val::Px(1.0),
            offset: Val::Auto,
            color: color::palettes::basic::YELLOW.into(),
        }
    }
}

#[derive(Component, Clone, Copy, PartialEq, Eq, Debug)]
enum ContentType {
    /// Marker component for the Text component that will display
    /// hover information. Used to keep from spawning tons of list items
    /// when hovering. The list is only used when the selection is locked.
    HoverInfo,

    /// Marker component for the interactable list of hovered entities.
    HoverList,

    /// Marks the button used to select and lock the list of hovered entities.
    SelectButton,

    /// Marker component for displaying the list of components for an entity.
    ComponentInfo,
}

///
/// The state of the inspector. Controls when to update and display information.
///
#[derive(Clone, Copy, PartialEq, Eq)]
enum State {
    None,
    Select,
    UpdateList,
    Locked,
}

#[derive(Resource, Clone)]
struct Data {
    hovered_window: Entity,
    hovered: Vec<Entity>,
    update_component_info_system: SystemId,
    hovered_updated: bool,
}

impl Data {
    fn new(update_component_info_system: SystemId) -> Self {
        Self {
            hovered_window: Entity::PLACEHOLDER,
            hovered: Vec::new(),
            update_component_info_system,
            hovered_updated: false,
        }
    }
}

#[derive(QueryData)]
#[query_data(mutable)]
struct NodeQuery {
    entity: Entity,
    component: &'static mut Node,
    computed: &'static ComputedNode,
    target: &'static ComputedNodeTarget,
    transform: &'static GlobalTransform,
}

fn toggle_display(
    inspector: Entity,
    parents: &Query<&Children>,
    content: &Query<(Entity, &ContentType)>,
    content_type: ContentType,
    display: Display,
    commands: &mut Commands,
) {
    let visibility = match display {
        Display::None => Visibility::Hidden,
        _ => Visibility::Visible,
    };

    let entities = get_content_type_entities(content, content_type);

    for entity in entities {
        if utility::is_descendant(entity, inspector, parents) {
            commands
                .entity(entity)
                .insert((
                    KeaNodeOverrides {
                        display: Some(display),
                        ..default()
                    },
                    visibility,
                ));
        }
    }
}

fn get_content_type_entities(
    content: &Query<(Entity, &ContentType)>,
    content_type: ContentType,
) -> Vec<Entity> {
    let mut result = Vec::<Entity>::new();

    for (entity, item) in content {
        if *item == content_type {
            result.push(entity);
        }
    }

    result
}

fn on_add(
    trigger: Trigger<OnAdd, KeaInspector>,
    mut commands: Commands,
) {
    commands.spawn(NodeOutline {
        inspector: trigger.target(),
    });
}

fn on_remove(
    trigger: Trigger<OnRemove, KeaInspector>,
    node_outlines: Query<(Entity, &NodeOutline)>,
    mut commands: Commands,
) {
    for (entity, node_outline) in node_outlines {
        if node_outline.inspector == trigger.target() {
            commands.entity(entity).despawn();
            break;
        }
    }
}

fn on_select_hovered(
    trigger: Trigger<KeaButtonClick>,
    children: Query<&ChildOf>,
    mut inspectors: Query<&mut KeaInspector>,
    mut commands: Commands,
) {
    for ancestor in children.iter_ancestors(trigger.target()) {
        let Ok(mut inspector) = inspectors.get_mut(ancestor) else {
            continue;
        };

        inspector.state = State::Select;
        break;
    }

    commands.kea_button_change_label(trigger.target(), "Selecting...");
}

fn on_clear_hovered(
    trigger: Trigger<KeaButtonClick>,
    content: Query<(Entity, &ContentType)>,
    parents: Query<&Children>,
    children: Query<&ChildOf>,
    mut inspectors: Query<&mut KeaInspector>,
    mut outlines: Query<(&NodeOutline, &mut Visibility)>,
    mut texts: Query<&mut Text>,
    mut commands: Commands,
) {
    let mut inspector = Entity::PLACEHOLDER;
    for ancestor in children.iter_ancestors(trigger.target()) {
        if let Ok(mut insp) = inspectors.get_mut(ancestor) {
            inspector = ancestor;
            insp.state = State::None;
            break;
        }
    }

    for (button, content_type) in &content {
        if *content_type != ContentType::SelectButton {
            continue;
        }

        if !utility::is_descendant(button, inspector, &parents) {
            continue;
        }

        commands.kea_button_change_label(button, "Select");
        break;
    }

    for (text_entity, content_type) in &content {
        if *content_type != ContentType::ComponentInfo {
            continue;
        }

        if !utility::is_descendant(text_entity, inspector, &parents) {
            continue;
        }

        let Ok(mut text) = texts.get_mut(text_entity) else {
            continue;
        };

        text.0.clear();
        break;
    }

    toggle_display(inspector, &parents, &content, ContentType::HoverList, Display::None, &mut commands);
    toggle_display(inspector, &parents, &content, ContentType::HoverInfo, Display::Flex, &mut commands);

    for (outline, mut visibility) in outlines.iter_mut() {
        if outline.inspector == inspector {
            *visibility = Visibility::Hidden;
            break;
        }
    }
}

fn update_hovered(
    mouse_motion: Res<AccumulatedMouseMotion>,
    windows: Query<(&Window, Entity, Option<&PrimaryWindow>)>,
    cameras: Query<&Camera>,
    nodes: Query<NodeQuery>,
    inspectors: Query<&KeaInspector>,
    mut data: ResMut<Data>,
) {
    // If no panels are present, then clear out the data and skip updates.
    if inspectors.is_empty() {
        return;
    }

    data.hovered_updated = false;

    // Mouse has not moved. Ignore any updates.
    if mouse_motion.delta == Vec2::ZERO {
        return;
    }

    data.hovered_updated = true;

    let mut hovered_window = Entity::PLACEHOLDER;
    let mut mouse_position = Vec2::ZERO;
    let mut is_primary = false;
    for (window, window_entity, primary_window) in windows {
        if let Some(mouse) = window.cursor_position() {
            hovered_window = window_entity;
            mouse_position = mouse;
            is_primary = primary_window.is_some();
            break;
        }
    }

    let mut hovered = Vec::<Entity>::new();
    for node in &nodes {
        let Some(camera_entity) = node.target.camera() else {
            continue;
        };

        let Ok(camera) = cameras.get(camera_entity) else {
            continue;
        };

        let is_valid = match camera.target {
            RenderTarget::Window(target) => {
                match target {
                    WindowRef::Primary => {
                        is_primary
                    },
                    WindowRef::Entity(entity) => {
                        entity == hovered_window
                    },
                }
            },
            _ => {
                false
            }
        };

        if !is_valid {
            continue;
        }

        let position = node.transform.translation().truncate();
        let size = node.computed.size();
        let rect = Rect::from_center_size(position, size);
        if rect.contains(mouse_position) {
            hovered.push(node.entity);
        }
    }

    data.hovered_window = hovered_window;
    data.hovered = hovered;
}

fn update_inspectors(
    mouse_button: Res<ButtonInput<MouseButton>>,
    parents: Query<&Children>,
    data: Res<Data>,
    content: Query<(Entity, &ContentType)>,
    inspectors: Query<(Entity, &mut KeaInspector)>,
    names: Query<&Name>,
    mut texts: Query<&mut Text>,
    mut commands: Commands,
) {
    for (entity, mut inspector) in inspectors {
        match inspector.state {
            State::Select => {
                // Lock the hovered nodes if the inspector is currently selecting
                if mouse_button.just_pressed(MouseButton::Left) {
                    inspector.state = State::UpdateList;
                    inspector.hovered = data.hovered.clone();

                    let current = if inspector.hovered.len() > 1 {
                        format!("{}", inspector.hovered.len())
                    } else {
                        format!("{}", inspector.hovered[0])
                    };

                    for (button, content_type) in &content {
                        if *content_type != ContentType::SelectButton {
                            continue;
                        }

                        if utility::is_descendant(button, entity, &parents) {
                            commands.kea_button_change_label(button, &format!("Select ({} selected)", current));
                        }
                    }

                    toggle_display(entity, &parents, &content, ContentType::HoverList, Display::Flex, &mut commands);
                    toggle_display(entity, &parents, &content, ContentType::HoverInfo, Display::None, &mut commands);
                }
            },
            State::UpdateList => {
                inspector.state = State::Locked;

                for (list, content_type) in &content {
                    if *content_type != ContentType::HoverList {
                        continue;
                    }

                    if !utility::is_descendant(list, entity, &parents) {
                        continue;
                    }

                    commands
                        .entity(list)
                        .insert(Visibility::Visible)
                        .remove::<Children>();

                    let mut items = Vec::<String>::new();
                    for hovered in &inspector.hovered {
                        if let Ok(name) = names.get(*hovered) {
                            items.push(format!("{} {hovered}", name.as_str()));
                        } else {
                            items.push(format!("Node {hovered}"));
                        }
                    }

                    commands
                        .entity(list)
                        .insert(KeaListLabelItems(items));

                    break;
                }
            },
            _ => {},
        }

        if data.hovered_updated && inspector.state != State::Locked {
            for (text_entity, content_type) in content {
                if *content_type != ContentType::HoverInfo {
                    continue;
                }

                if !utility::is_descendant(text_entity, entity, &parents) {
                    continue;
                }

                let Ok(mut text) = texts.get_mut(text_entity) else {
                    continue;
                };

                let mut info = String::new();
                for hovered in &data.hovered {
                    info += &format!("Node {hovered}\n");
                }
                info.pop();

                *text = info.into();
            }
        }
    }
}

fn on_select_list_item(
    trigger: Trigger<KeaListSelect>,
    children: Query<&ChildOf>,
    data: ResMut<Data>,
    mut inspectors: Query<&mut KeaInspector>,
    mut commands: Commands,
) {
    let Some(selected) = data.hovered.get(trigger.event().index) else {
        return;
    };

    let Some(root) = children.iter_ancestors(trigger.target()).find(|&value| inspectors.contains(value)) else {
        return;
    };

    let Ok(mut inspector) = inspectors.get_mut(root) else {
        return;
    };

    inspector.selected = *selected;
    commands.run_system(data.update_component_info_system);
}

fn on_hover_list_item(
    trigger: Trigger<KeaListHover>,
    outlines: Query<(Entity, &NodeOutline)>,
    inspectors: Query<&KeaInspector>,
    children: Query<&ChildOf>,
    mut nodes: Query<(&mut Node, &ComputedNode, &GlobalTransform)>,
    mut commands: Commands,
) {
    let mut inspector = Entity::PLACEHOLDER;
    let mut hovered = Entity::PLACEHOLDER;
    for ancestor in children.iter_ancestors(trigger.target()) {
        let Ok(insp) = inspectors.get(ancestor) else {
            continue;
        };

        inspector = ancestor;
        if let Some(value) = insp.hovered.get(trigger.event().index) {
            hovered = *value;
        }

        break;
    }

    let bounds = if let Ok((_, computed_node, transform)) = nodes.get(hovered) {
        Rect::from_center_size(transform.translation().truncate(), computed_node.size())
    } else {
        Rect::EMPTY
    };

    for (outline_entity, outline) in outlines {
        if outline.inspector != inspector {
            continue;
        }

        let Ok((mut node, _, _)) = nodes.get_mut(outline_entity) else {
            continue;
        };

        node.left = Val::Px(bounds.min.x);
        node.top = Val::Px(bounds.min.y);
        node.width = Val::Px(bounds.width());
        node.height = Val::Px(bounds.height());

        commands
            .entity(outline_entity)
            .insert(Visibility::Visible);
    }
}

/// Needs to be an exclusive system as this function needs access to
/// component information.
fn update_component_info(world: &mut World) {
    struct InspectorSelectMap {
        inspector: Entity,
        selected: Entity,
        info: Option<String>,
    }

    // Grab a mapping of inspector -> selected objects
    let mut selected: Vec<InspectorSelectMap> = {
        let mut result = Vec::new();

        let mut inspectors = world.query::<(Entity, &KeaInspector)>();
        for (entity, inspector) in inspectors.iter(world) {
            result.push(InspectorSelectMap {
                inspector: entity,
                selected: inspector.selected,
                info: None,
            });
        }

        result
    };

    // Gather all component information for each valid selected entity
    for item in &mut selected {
        if item.selected == Entity::PLACEHOLDER {
            continue;
        }

        let Ok(components) = world.inspect_entity(item.selected) else {
            continue;
        };

        let mut names: Vec<String> = Vec::new();
        for component in components {
            let name = component.name();
            let tokens: Vec<&str> = name.rsplit("::").collect();
            if tokens.is_empty() {
                names.push(name.to_string());
            } else {
                names.push(tokens[0].to_string());
            }
        }

        names.sort_by(|a, b| a.to_lowercase().cmp(&b.to_lowercase()));

        let mut info = String::new();
        for name in names {
            info += &name;
            info += "\n";
        }
        // Remove the last '\n' character
        info.pop();

        item.info = Some(info);
    }

    // Update the owning Text components through a system-like operation.
    let mut system_state: SystemState<(
        Query<(Entity, &mut Text, &ContentType)>,
        Query<&ChildOf>,
    )> = SystemState::new(world);

    let (texts, children) = system_state.get_mut(world);
    for (entity, mut text, content_type) in texts {
        if *content_type != ContentType::ComponentInfo {
            continue;
        }

        let ancestors: Vec<Entity> = children.iter_ancestors(entity).collect();

        for select in &selected {
            if !ancestors.contains(&select.inspector) {
                continue;
            }

            let Some(info) = &select.info else {
                continue;
            };

            *text = info.clone().into();
        }
    }

    system_state.apply(world);
}
