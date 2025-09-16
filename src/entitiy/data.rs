use lstalk::prelude::*;

#[derive(Debug)]
pub enum EntityPropertyData {
    None,
    Boolean(bool),
    Integer(i64),
    Decimal(f64),
}

impl From<DataType> for EntityPropertyData {
    fn from(value: DataType) -> Self {
        match value {
            DataType::Boolean => Self::Boolean(false),
            DataType::Number => Self::Integer(0),
            DataType::Decimal => Self::Decimal(0.0),
            _ => Self::None,
        }
    }
}
