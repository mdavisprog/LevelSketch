use lstalk::prelude::*;

pub enum EntityPropertyData {
    None,
    Decimal(f64),
}

impl EntityPropertyData {
    pub fn set_decimal(&mut self, value: f64) -> bool {
        match self {
            Self::Decimal(_) => {
                *self = Self::Decimal(value);
                true
            },
            _ => false,
        }
    }
}

impl From<DataType> for EntityPropertyData {
    fn from(value: DataType) -> Self {
        match value {
            DataType::Decimal => Self::Decimal(0.0),
            _ => Self::None,
        }
    }
}
