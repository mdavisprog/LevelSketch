use bevy::prelude::*;
use kea::prelude::*;
use crate::{
    entitiy::EntityProperties,
    gui::tools::{
        tools::Tools,
        types::{
            TypesTool,
            TypeSelected,
        },
    },
    project::LSPServiceResource,
    tools::selection::{
        Selection,
        SelectionChanged,
    },
};
use super::{
    transform::{
        PropertyTransform,
        PropertyTransformField,
    },
    property::spawn_property,
};

#[derive(Component)]
#[require(Tools)]
pub struct PropertiesTool {
    entity: Entity,
}

impl PropertiesTool {
    pub fn bundle() -> impl Bundle {(
        Self {
            entity: Entity::PLACEHOLDER,
        },
        children![
            no_entity(),
        ],
    )}
}

#[derive(Event)]
pub struct Refresh;

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
struct PropertiesToolBar {
    _private: (),
}

impl PropertiesToolBar {
    fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        children![
            (
                KeaButton::image_bundle(
                    "kea://icons/plus.svg#image12x12",
                    on_add_property,
                ),
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            column_gap: Val::Px(kea::style::properties::ROW_GAP),
            ..default()
        }
    }
}

pub(super) fn build(app: &mut App) {
    app
        .add_systems(Update, on_transform_changed)
        .add_observer(on_selection_changed)
        .add_observer(refresh);
}

fn no_entity() -> impl Bundle {(
    KeaLabel::bundle("No Entity"),
    Node {
        align_self: AlignSelf::Center,
        justify_self: JustifySelf::Center,
        ..default()
    },
)}

fn on_selection_changed(
    _: Trigger<SelectionChanged>,
    mut commands: Commands,
) {
    commands.trigger(Refresh);
}

fn refresh(
    _: Trigger<Refresh>,
    selection: Res<Selection>,
    transforms: Query<&Transform>,
    entity_properties: Query<&EntityProperties>,
    mut tools: Query<(Entity, &mut PropertiesTool)>,
    mut commands: Commands,
) {
    for (tool_entity, mut tool) in &mut tools {
        commands
            .entity(tool_entity)
            .despawn_related::<Children>();

        let entity = if selection.world().is_empty() {
            Entity::PLACEHOLDER
        } else {
            *selection.world().get(0).unwrap()
        };

        let Ok(entity_properties) = entity_properties.get(entity) else {
            commands
                .entity(tool_entity)
                .with_child(no_entity());

            return;
        };

        tool.entity = entity;

        let Ok(transform) = transforms.get(entity) else {
            warn!("Failed to get Transform component from entity {entity}.");
            return;
        };

        let mut property_entities = Vec::<Entity>::new();
        for (_name, property) in &entity_properties.properties {
            let id = spawn_property(
                entity,
                property,
                &mut commands
            );

            property_entities.push(id);
        }

        // First property should be the transform.
        let mut entity_ref = commands.entity(tool_entity);
        entity_ref
            .with_child(PropertiesToolBar::bundle())
            .with_child(KeaExpander::bundle_with_header(
                "Transform",
                PropertyTransform::bundle(entity, transform),
            ))
            .add_children(&property_entities);
    }
}

fn on_transform_changed(
    transforms: Query<(Entity, &Transform), Changed<Transform>>,
    tools: Query<(Entity, &PropertiesTool)>,
    parents: Query<&Children>,
    transform_properties: Query<&PropertyTransform>,
    transform_fields: Query<&PropertyTransformField>,
    mut commands: Commands,
) {
    for (entity, transform) in transforms {
        for (tool_entity, tool) in tools {
            if tool.entity != entity {
                continue;
            }

            for child in parents.iter_descendants(tool_entity) {
                // Look for the PropertyTransform component. The children of this component
                // contains the fields needed for updating the transform.
                if !transform_properties.contains(child) {
                    continue;
                }

                for property_child in parents.iter_descendants(child) {
                    let Ok(field) = transform_fields.get(property_child) else {
                        continue;
                    };

                    let Ok(field_parent) = parents.get(property_child) else {
                        continue;
                    };

                    let (x, y, z) = match field {
                        PropertyTransformField::Translation => {(
                            transform.translation.x.to_string(),
                            transform.translation.y.to_string(),
                            transform.translation.z.to_string(),
                        )},
                        PropertyTransformField::Rotation => {
                            let (
                                pitch,
                                yaw,
                                roll,
                            ) = transform.rotation.to_euler(EulerRot::XYZ);

                            (
                                pitch.to_degrees().to_string(),
                                yaw.to_degrees().to_string(),
                                roll.to_degrees().to_string(),
                            )
                        },
                        PropertyTransformField::Scale => {(
                            transform.scale.x.to_string(),
                            transform.scale.y.to_string(),
                            transform.scale.z.to_string(),
                        )},
                    };

                    commands.kea_property_set_value(field_parent[0], x);
                    commands.kea_property_set_value(field_parent[1], y);
                    commands.kea_property_set_value(field_parent[2], z);
                }

                break;
            }
        }
    }
}

fn on_add_property(
    _: Trigger<KeaButtonClick>,
    mut commands: Commands,
) {
    struct TypeSelected;

    commands.kea_popup_open(
        (
            Node {
                width: Val::Percent(100.0),
                height: Val::Percent(100.0),
                ..default()
            },
            BackgroundColor(kea::style::colors::BACKGROUND),
            TypesTool::bundle(),
            KeaObservers::<TypeSelected>::new(vec![
                Observer::new(on_type_selected),
            ]),
        ),
        KeaPopupPosition::AtMouse,
        KeaPopupSize::Fixed(Vec2::new(300.0, 400.0)),
    );
}

fn on_type_selected(
    trigger: Trigger<TypeSelected>,
    lsp_resource: Res<LSPServiceResource>,
    tools: Query<(Entity, &PropertiesTool)>,
    mut entity_properties: Query<&mut EntityProperties>,
    mut commands: Commands,
) {
    let event = trigger.event();

    let Some(symbols) = lsp_resource.symbols() else {
        panic!("No symbol table from LSP after type selection.");
    };

    let Some(symbol) = symbols.get(&event.name) else {
        panic!("Failed to find symbol {}.", event.name);
    };

    for (tool_entity, tool) in tools {
        let Ok(mut entity_props) = entity_properties.get_mut(tool.entity) else {
            panic!("Entity {} does not have an EntityProperties component.", tool.entity);
        };

        if entity_props.contains(symbol.name()) {
            continue;
        }

        let property = entity_props.add(symbol);
        let id = spawn_property(
            tool.entity,
            property,
            &mut commands,
        );

        commands
            .entity(tool_entity)
            .add_child(id);
    }
}
