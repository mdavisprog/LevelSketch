use bevy::prelude::*;
use crate::controls::text::{
    KeaTextInputFormat,
    KeaTextInputFormatNumber,
};
use super::{
    systems::on_decimal_confirm,
    text::KeaPropertyText,
};

#[derive(Component)]
pub struct KeaPropertyDecimal {
    _private: (),
}

impl KeaPropertyDecimal {
    pub fn bundle(label: &str, value: f64) -> impl Bundle {(
        Self {
            _private: (),
        },
        KeaPropertyText::bundle_with_format_and_callback(
            label,
            &value.to_string(),
            KeaTextInputFormat::Numbers(KeaTextInputFormatNumber {
                precision: 4,
            }),
            on_decimal_confirm,
        )
    )}
}
