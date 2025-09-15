use bevy::prelude::*;
use kea::prelude::*;

#[derive(Component)]
pub enum PropertyTransformField {
    Translation,
    Rotation,
    Scale,
}

#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct PropertyTransform {
    entity: Entity,
}

impl PropertyTransform {
    pub fn bundle(
        entity: Entity,
        transform: &Transform,
    ) -> impl Bundle {(
        Self {
            entity,
        },
        children![
            (
                Self::bundle_field::<PropertyTransformFieldTranslation>(
                    PropertyTransformField::Translation,
                    "Translation",
                    transform.translation
                ),
            ),
            (
                Self::bundle_field::<PropertyTransformFieldRotation>(
                    PropertyTransformField::Rotation,
                    "Rotation",
                    to_vec3(transform.rotation),
                ),
            ),
            (
                Self::bundle_field::<PropertyTransformFieldScale>(
                    PropertyTransformField::Scale,
                    "Scale",
                    transform.scale,
                ),
            ),
        ]
    )}

    fn bundle_field<T: Send + Sync + 'static>(
        field: PropertyTransformField,
        label: &str,
        value: Vec3,
    ) -> impl Bundle {(
        Node {
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(kea::style::properties::ROW_GAP),
            ..default()
        },
        children![
            (
                KeaLabel::bundle(label),
                Node {
                    align_self: AlignSelf::Center,
                    ..default()
                },
            ),
            (
                field,
                KeaPropertyVector3::bundle(value),
                KeaObservers::<T>::new(vec![
                    Observer::new(on_confirm),
                ]),
            ),
        ],
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(kea::style::properties::ROW_GAP),
            ..default()
        }
    }
}

struct PropertyTransformFieldTranslation;
struct PropertyTransformFieldRotation;
struct PropertyTransformFieldScale;

fn to_vec3(rotation: Quat) -> Vec3 {
    let (pitch, yaw, roll) = rotation.to_euler(EulerRot::XYZ);
    Vec3::new(
        pitch.to_degrees(),
        yaw.to_degrees(),
        roll.to_degrees(),
    )
}

fn on_confirm(
    trigger: Trigger<KeaPropertyChanged>,
    fields: Query<&PropertyTransformField>,
    children: Query<&ChildOf>,
    transform_properties: Query<&PropertyTransform>,
    mut transforms: Query<&mut Transform>,
) {
    let KeaPropertyData::Vector3(value) = trigger.event().data else {
        panic!("Invalid property data.");
    };

    let Ok(field) = fields.get(trigger.target()) else {
        panic!("PropertyTransformField component not found.");
    };

    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(property) = transform_properties.get(parent) else {
            continue;
        };

        let Ok(mut transform) = transforms.get_mut(property.entity) else {
            panic!("Entity {} does not have a Transform component.", property.entity);
        };

        match field {
            PropertyTransformField::Translation => {
                transform.translation = value;
            },
            PropertyTransformField::Rotation => {
                transform.rotation = Quat::from_euler(
                    EulerRot::XYZ,
                    value.x.to_radians(),
                    value.y.to_radians(),
                    value.z.to_radians(),
                );
            },
            PropertyTransformField::Scale => {
                transform.scale = value;
            },
        }

        break;
    }
}
