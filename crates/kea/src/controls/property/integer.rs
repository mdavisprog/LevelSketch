use bevy::prelude::*;
use crate::controls::text::{
    KeaTextInputFormat,
    KeaTextInputFormatNumber,
    KeaTextInputFormatNumberType,
};
use super::{
    systems::on_integer_confirm,
    text::KeaPropertyText,
};

#[derive(Component)]
pub struct KeaPropertyInteger {
    value: i64,
}

impl KeaPropertyInteger {
    pub fn bundle(label: &str, value: i64) -> impl Bundle {(
        Self {
            value,
        },
        KeaPropertyText::bundle_with_format_and_callback(
            label,
            &value.to_string(),
            KeaTextInputFormat::Numbers(KeaTextInputFormatNumber {
                precision: 0,
                number_type: KeaTextInputFormatNumberType::Integer,
            }),
            on_integer_confirm,
        ),
    )}

    pub fn value(&self) -> i64 {
        self.value
    }
}
