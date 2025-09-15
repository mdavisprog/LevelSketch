use bevy::prelude::*;
use crate::entitiy::{
    EntityProperty,
    EntityProperties,
    EntityPropertyData,
};
use kea::prelude::*;
use lstalk::prelude::*;

pub fn spawn_property(
    entity: Entity,
    property: &EntityProperty,
    commands: &mut Commands,
) -> Entity {
    let parent = commands.spawn(
        Node {
            width: Val::Percent(100.0),
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(kea::style::properties::ROW_GAP),
            ..default()
        },
    )
    .id();

    for (_name, sub_property) in property.properties() {
        let id = spawn_field(sub_property, commands);
        if id != Entity::PLACEHOLDER {
            commands
                .entity(parent)
                .add_child(id);
        }
    }

    let id = commands.spawn(Property::bundle(entity, property.name())).id();
    commands.kea_expander_add_contents(id, [parent].to_vec());

    id
}

#[derive(Component)]
struct Property {
    entity: Entity,
}

impl Property {
    fn bundle(
        entity: Entity,
        label: &str,
    ) -> impl Bundle {(
        Self {
            entity,
        },
        KeaExpander::bundle_with_header(
            label,
            (),
        ),
    )}
}

#[derive(Component)]
struct PropertyField {
    path: SymbolPath,
}

fn spawn_field(
    property: &EntityProperty,
    commands: &mut Commands,
) -> Entity {
    match property.data() {
        EntityPropertyData::Decimal(value) => {
            commands.spawn((
                KeaPropertyDecimal::bundle(
                    property.name(),
                    *value,
                ),
                PropertyField {
                    path: property.path().clone(),
                },
            ))
            .observe(on_decimal_changed)
            .id()
        },
        EntityPropertyData::None => Entity::PLACEHOLDER,
    }
}

fn on_decimal_changed(
    trigger: Trigger<KeaPropertyChanged>,
    fields: Query<&PropertyField>,
    children: Query<&ChildOf>,
    properties: Query<&Property>,
    mut entity_properties: Query<&mut EntityProperties>,
) {
    let Ok(field) = fields.get(trigger.target()) else {
        panic!("PropertyField component not found on decimal property.");
    };

    let KeaPropertyData::Decimal(value) = trigger.event().data else {
        panic!("Incorrect data for decimal property.");
    };

    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(property) = properties.get(parent) else {
            continue;
        };

        let Ok(mut entity_props) = entity_properties.get_mut(property.entity) else {
            panic!("Failed to find EntityProperties for entity {}.", property.entity);
        };

        let Some(property) = entity_props.get_mut(&field.path) else {
            panic!("Failed to find property for {}.", field.path);
        };

        property.data_mut().set_decimal(value);
    }
}
