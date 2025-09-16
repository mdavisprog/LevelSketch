use std::collections::HashMap;

#[derive(Debug, Default, PartialEq, Eq, Hash, Clone, Copy)]
pub enum DataType {
    #[default]
    None,
    Boolean,
    Number,
    Decimal,
    String,
    Object,
    Array,
}

/// A database that maps a file extension to all possible data types.
pub struct DataTypeDB {
    /// Languages with possible string representations per data type.
    languages: Vec<DataTypeLanguage>,
}

impl DataTypeDB {
    pub fn new() -> Self {
        Self {
            languages: Vec::new(),
        }
    }

    pub fn get_data_type(&self, extension: &str, token: &String) -> DataType {
        let ext = extension.to_string();
        for lang in &self.languages {
            if lang.is_extension(&ext) {
                return lang.get_data_type(token);
            }
        }

        DataType::None
    }
}

// TODO: Read from a JSON file for full customization.
impl Default for DataTypeDB {
    fn default() -> Self {
        let cpp = DataTypeLanguage {
            extensions: vec![
                format!("h"),
                format!("hpp"),
                format!("c"),
                format!("cpp"),
            ],
            map: DataTypeMap::from([
                (DataType::Boolean, vec![format!("bool")]),
                (DataType::Number, vec![
                    format!("int"),
                    format!("long"),
                    format!("char"),
                    format!("short"),
                ]),
                (DataType::Decimal, vec![
                    format!("float"),
                    format!("double"),
                ]),
                (DataType::String, vec![
                    format!("string"),
                    format!("std::string"),
                ]),
            ]),
        };

        Self {
            languages: vec![
                cpp,
            ],
        }
    }
}

type DataTypeMap = HashMap<DataType, Vec<String>>;

struct DataTypeLanguage {
    extensions: Vec<String>,
    map: DataTypeMap,
}

impl DataTypeLanguage {
    fn new() -> Self {
        Self {
            extensions: Vec::new(),
            map: DataTypeMap::new(),
        }
    }

    fn is_extension(&self, extension: &String) -> bool {
        self.extensions.contains(extension)
    }

    fn get_data_type(&self, token: &String) -> DataType {
        for (data_type, tokens) in &self.map {
            if tokens.contains(token) {
                return *data_type;
            }
        }

        DataType::None
    }
}
