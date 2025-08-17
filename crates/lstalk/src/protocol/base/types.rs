use serde::{
    Deserialize,
    Serialize,
};
use serde_json::{
    Map,
    Number,
    Value,
};
use std::collections::HashMap;

pub type Boolean = bool;
pub type Integer = i64;
pub type UInteger = u64;
pub type Decimal = f64;

pub type LSPObject = HashMap<String, LSPAny>;
pub type LSPArray = Vec<LSPAny>;

#[derive(Serialize, Deserialize, Clone)]
#[serde(untagged)]
pub enum LSPAny {
    Null,
    Object(LSPObject),
    Array(LSPArray),
    String(String),
    Integer(Integer),
    UInteger(UInteger),
    Decimal(Decimal),
    Boolean(Boolean),
}

impl From<Value> for LSPAny {
    fn from(value: Value) -> Self {
        match value {
            Value::Array(array) => {
                let mut vec = LSPArray::with_capacity(array.capacity());

                for item in array {
                    vec.push(item.into());
                }

                Self::Array(vec)
            },
            Value::Bool(bool) => Self::Boolean(bool),
            Value::Null => Self::Null,
            Value::Number(number) => number.into(),
            Value::Object(object) => {
                let mut map = LSPObject::with_capacity(object.len());

                for (key, value) in object {
                    map.insert(key, value.into());
                }

                Self::Object(map)
            },
            Value::String(string) => Self::String(string),
        }
    }
}

impl LSPAny {
    pub fn kind(&self) -> &str {
        match self {
            Self::Array(_) => "Array",
            Self::Boolean(_) => "Boolean",
            Self::Decimal(_) => "Decimal",
            Self::Integer(_) => "Integer",
            Self::Null => "Null",
            Self::Object(_) => "Object",
            Self::String(_) => "String",
            Self::UInteger(_) => "UInteger",
        }
    }
}

impl From<LSPAny> for Value {
    fn from(value: LSPAny) -> Self {
        match value {
            LSPAny::Array(array) => {
                let mut result = Vec::<Value>::new();
                for item in array {
                    result.push(item.into());
                }

                Value::Array(result)
            },
            LSPAny::Boolean(boolean) => Value::Bool(boolean),
            LSPAny::Decimal(decimal) => {
                if let Some(float) = Number::from_f64(decimal) {
                    Value::Number(float)
                } else {
                    Value::Null
                }
            },
            LSPAny::Integer(integer) => Value::Number(integer.into()),
            LSPAny::Null => Value::Null,
            LSPAny::Object(object) => {
                let mut result = Map::<String, Value>::new();
                for (k, v) in object {
                    result.insert(k, v.into());
                }

                Value::Object(result)
            },
            LSPAny::String(string) => Value::String(string),
            LSPAny::UInteger(uinteger) => Value::Number(uinteger.into()),
        }
    }
}

impl std::fmt::Display for LSPAny {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Array(value) => {
                let mut collection = String::new();
                for item in value {
                    collection += &format!("{item}, ");
                }
                collection.drain((collection.len() - 2)..);
                write!(f, "{collection}")
            },
            Self::Boolean(value) => write!(f, "{value}"),
            Self::Decimal(value) => write!(f, "{value}"),
            Self::Integer(value) => write!(f, "{value}"),
            Self::Null => write!(f, "null"),
            Self::Object(value) => {
                let mut collection = String::new();
                for (k, v) in value {
                    collection += &format!("{k}: {v}, ");
                }
                collection.drain((collection.len() - 2)..0);
                write!(f, "{collection}")
            },
            Self::String(value) => write!(f, "{value}"),
            Self::UInteger(value) => write!(f, "{value}"),
        }
    }
}

impl PartialEq for LSPAny {
    fn eq(&self, other: &Self) -> bool {
        match self {
            Self::Array(array) => {
                if let LSPAny::Array(other_array) = other {
                    *array == *other_array
                } else {
                    false
                }
            },
            Self::Boolean(boolean) => {
                if let LSPAny::Boolean(other_boolean) = other {
                    *boolean == *other_boolean
                } else {
                    false
                }
            },
            Self::Decimal(decimal) => {
                if let LSPAny::Decimal(other_decimal) = other {
                    *decimal == *other_decimal
                } else {
                    false
                }
            },
            Self::Integer(integer) => {
                if let LSPAny::Integer(other_integer) = other {
                    *integer == *other_integer
                } else {
                    false
                }
            },
            Self::Null => {
                *other == LSPAny::Null
            },
            Self::Object(object) => {
                if let LSPAny::Object(other_object) = other {
                    *object == *other_object
                } else {
                    false
                }
            },
            Self::String(string) => {
                if let LSPAny::String(other_string) = other {
                    *string == *other_string
                } else {
                    false
                }
            },
            Self::UInteger(uinteger) => {
                if let LSPAny::UInteger(other_uinteger) = other {
                    *uinteger == *other_uinteger
                } else {
                    false
                }
            },
        }
    }
}

impl Eq for LSPAny {}

impl From<Number> for LSPAny {
    fn from(value: Number) -> Self {
        if value.is_f64() {
            if let Some(n) = value.as_f64() {
                return Self::Decimal(n);
            }
        } else if value.is_i64() {
            if let Some(n) = value.as_i64() {
                return Self::Integer(n);
            }
        } else if value.is_u64() {
            if let Some(n) = value.as_u64() {
                return Self::UInteger(n);
            }
        }

        LSPAny::Null
    }
}
