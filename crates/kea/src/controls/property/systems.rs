use bevy::prelude::*;
use crate::controls::{
    checkbox::{
        KeaCheckboxClicked,
        KeaCheckboxState,
    },
    text::{
        KeaTextInputCommands,
        KeaTextInput,
        KeaTextInputConfirm,
        KeaTextInputUnfocus,
    },
};
use super::{
    boolean::KeaPropertyBoolean,
    decimal::KeaPropertyDecimal,
    events::{
        KeaPropertyChanged,
        KeaPropertyData,
    },
    integer::KeaPropertyInteger,
    text::KeaPropertyText,
    vector::{
        KeaPropertyVector3,
        KeaPropertyVector3Field,
    },
};

pub(super) fn on_decimal_confirm(
    trigger: Trigger<KeaTextInputConfirm>,
    children: Query<&ChildOf>,
    decimals: Query<&KeaPropertyDecimal>,
    mut texts: Query<&mut KeaPropertyText>,
    mut commands: Commands,
) {
    let value = match trigger.event().text.parse::<f64>() {
        Ok(result) => result,
        Err(error) => panic!("Failed to parse 'f64': {error:?}"),
    };

    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(mut text) = texts.get_mut(parent) else {
            continue;
        };

        text.value = value.to_string();
        break;
    }

    for parent in children.iter_ancestors(trigger.target()) {
        if !decimals.contains(parent) {
            continue;
        }

        commands.trigger_targets(KeaPropertyChanged {
            data: KeaPropertyData::Decimal(value),
        }, parent);

        break;
    }
}

pub(super) fn on_text_confirm(
    trigger: Trigger<KeaTextInputConfirm>,
    children: Query<&ChildOf>,
    mut texts: Query<&mut KeaPropertyText>,
    mut commands: Commands,
) {
    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(mut text) = texts.get_mut(parent) else {
            continue;
        };

        let value = trigger.event().text.clone();
        text.value = value.clone();

        commands.trigger_targets(KeaPropertyChanged {
            data: KeaPropertyData::Text(value),
        }, parent);

        break;
    }
}

pub(super) fn on_confirm_vector3(
    trigger: Trigger<KeaPropertyChanged>,
    fields: Query<&KeaPropertyVector3Field>,
    children: Query<&ChildOf>,
    mut properties: Query<&mut KeaPropertyVector3>,
    mut commands: Commands,
) {
    let field = fields.get(trigger.target()).expect(&format!(
        "Failed to find KeaPropertyVector3Field for entity {}.", trigger.target()
    ));

    let child = children.get(trigger.target()).expect(&format!(
        "Failed to get ChildOf for entity {}.", trigger.target()
    ));

    let mut property = properties.get_mut(child.parent()).expect(&format!(
        "Failed to get KeaPropertyVector3 for entity: {}", trigger.target(),
    ));

    let KeaPropertyData::Decimal(value) = trigger.event().data else {
        panic!("KeaPropertyVector3 input field is not a decimal.");
    };

    property.value = match field {
        KeaPropertyVector3Field::X => Vec3::new(
            value as f32,
            property.value.y,
            property.value.z,
        ),
        KeaPropertyVector3Field::Y => Vec3::new(
            property.value.x,
            value as f32,
            property.value.z,
        ),
        KeaPropertyVector3Field::Z => Vec3::new(
            property.value.x,
            property.value.y,
            value as f32,
        ),
    };

    commands.trigger_targets(KeaPropertyChanged {
        data: KeaPropertyData::Vector3(property.value),
    }, child.parent());
}

pub(super) fn on_checkbox(
    trigger: Trigger<KeaCheckboxClicked>,
    mut properties: Query<&mut KeaPropertyBoolean>,
    mut commands: Commands,
) {
    let Ok(mut property) = properties.get_mut(trigger.target()) else {
        panic!("Failed to get KeaPropertyBoolean component.");
    };

    property.value = match trigger.event().state {
        KeaCheckboxState::Checked => true,
        KeaCheckboxState::Unchecked => false,
    };

    commands.trigger_targets(KeaPropertyChanged {
        data: KeaPropertyData::Boolean(property.value),
    }, trigger.target());
}

pub(super) fn on_integer_confirm(
    trigger: Trigger<KeaTextInputConfirm>,
    children: Query<&ChildOf>,
    integers: Query<&KeaPropertyInteger>,
    mut commands: Commands,
) {
    let value = match trigger.event().text.parse::<i64>() {
        Ok(result) => result,
        Err(error) => panic!("Failed to parse 'i64': {error:?}"),
    };

    for parent in children.iter_ancestors(trigger.target()) {
        if !integers.contains(parent) {
            continue;
        }

        commands.trigger_targets(KeaPropertyChanged {
            data: KeaPropertyData::Integer(value),
        }, parent);

        break;
    }
}

pub(super) fn on_unfocus(
    trigger: Trigger<KeaTextInputUnfocus>,
    text_properties: Query<&KeaPropertyText>,
    text_inputs: Query<&KeaTextInput>,
    children: Query<&ChildOf>,
    mut commands: Commands,
) {
    if !text_inputs.contains(trigger.target()) {
        panic!("Receiving a KeaTextInputUnfocus for invalid KeaTextInput component.");
    };

    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(property) = text_properties.get(parent) else {
            continue;
        };

        commands.kea_text_input_set_text(trigger.target(), property.value.clone());
        break;
    }
}
