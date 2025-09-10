use crate::protocol::document;
use super::{
    data::DataType,
    path::SymbolPath,
    semantic::{
        SemanticTokenModifiers,
        SemanticTokenType,
    },
    table::SymbolTable,
};

/// Represents the name, type, and other attributes of a symbol. A symbol also has a kind, which
/// describes if the symbol is a class, field, etc.
///
/// This is the type users will be interacting with when receiving responses from the language
/// server process.
pub struct Symbol {
    /// The name for this symbol from the source code.
    pub(crate) name: String,

    /// Describes what the symbol represents in the source code.
    pub(crate) kind: SymbolKind,

    /// Describes what programming construct this symbol represents like class, enum, struct,
    /// variable, etc.
    pub(crate) semantic_type: SemanticTokenType,

    /// Describes extra information about this symbol such as static, abstract, etc.
    pub(crate) modifiers: SemanticTokenModifiers,

    /// Describes how data should be represented for this symbol.
    pub(crate) data_type: DataType,

    /// The fully-qualified path to this symbol.
    pub(crate) path: SymbolPath,

    /// List of symbols contained within this scope.
    symbols: SymbolTable,
}

impl Symbol {
    pub fn new(
        name: String,
        kind: SymbolKind,
    ) -> Self {
        let path = SymbolPath::new(name.clone());
        Self {
            name,
            kind,
            symbols: SymbolTable::new(),
            semantic_type: SemanticTokenType::Type,
            modifiers: SemanticTokenModifiers::new(),
            data_type: DataType::None,
            path,
        }
    }

    pub fn name(&self) -> &String {
        &self.name
    }

    pub fn kind(&self) -> SymbolKind {
        self.kind
    }

    pub fn symbols(&self) -> &SymbolTable {
        &self.symbols
    }

    pub fn data_type(&self) -> DataType {
        self.data_type
    }

    pub fn path(&self) -> &SymbolPath {
        &self.path
    }

    pub(crate) fn add(&mut self, mut symbol: Symbol) -> &mut Self {
        symbol.path.prepend(&self.path);
        self.symbols.insert(symbol.name.clone(), symbol);
        self
    }
}

impl From<document::SymbolInformation> for Symbol {
    fn from(value: document::SymbolInformation) -> Self {
        Self::new(value.name, value.kind.into())
    }
}

impl Symbol {
    pub fn to_string(&self) -> String {
        self.to_string_recurse(self, 0)
    }

    fn to_string_recurse(&self, symbol: &Self, level: usize) -> String {
        let indent = " ".repeat(level);
        let mut result = format!("{indent}{} => {:?} {:?}",
            symbol.name,
            symbol.kind,
            symbol.data_type,
        );

        for (_, child) in &symbol.symbols {
            result += "\n";
            result += &self.to_string_recurse(child, level + 4);
        }

        result
    }
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum SymbolKind {
    None,
    Class,
    Field,
}

impl From<document::SymbolKind> for SymbolKind {
    fn from(value: document::SymbolKind) -> Self {
        match value {
            document::SymbolKind::Class => Self::Class,
            document::SymbolKind::Field => Self::Field,
            _ => Self::None,
        }
    }
}
