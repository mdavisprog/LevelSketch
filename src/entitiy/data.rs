use lstalk::prelude::*;

#[derive(Debug)]
pub enum EntityPropertyData {
    None,
    Boolean(bool),
    Decimal(f64),
}

impl From<DataType> for EntityPropertyData {
    fn from(value: DataType) -> Self {
        match value {
            DataType::Boolean => Self::Boolean(false),
            DataType::Decimal => Self::Decimal(0.0),
            _ => Self::None,
        }
    }
}
