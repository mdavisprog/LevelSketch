/// The user facing version of the SemanticTokenType from the server.
#[derive(Debug)]
pub enum SemanticTokenType {
    Namespace,
    Type,
    Class,
    Enum,
    Struct,
    Variable,
    Property,
    EnumMember,
    Function,
    Method,
    Keyword,
    Modifier,
    Comment,
    String,
    Number,
    Unknown(String),
}

impl From<&str> for SemanticTokenType {
    fn from(value: &str) -> Self {
        match value {
            "namespace" => Self::Namespace,
            "type" => Self::Type,
            "class" => Self::Class,
            "enum" => Self::Enum,
            "struct" => Self::Struct,
            "variable" => Self::Variable,
            "property" => Self::Property,
            "enumMember" => Self::EnumMember,
            "function" => Self::Function,
            "method" => Self::Method,
            "keyword" => Self::Keyword,
            "modifier" => Self::Modifier,
            "comment" => Self::Comment,
            "string" => Self::String,
            "number" => Self::Number,
            _ => Self::Unknown(value.to_string()),
        }
    }
}

#[derive(Debug, PartialEq, Eq)]
pub enum SemanticTokenModifier {
    Declaration,
    Definition,
    ReadOnly,
    Static,
    Deprecated,
    Abstract,
    Async,
    Modification,
    Documentation,
    DefaultLibrary,
    Unknown(String),
}

impl From<&str> for SemanticTokenModifier {
    fn from(value: &str) -> Self {
        match value {
            "declaration" => Self::Declaration,
            "definition" => Self::Definition,
            "readonly" => Self::ReadOnly,
            "static" => Self::Static,
            "deprecated" => Self::Deprecated,
            "abstract" => Self::Abstract,
            "async" => Self::Async,
            "modification" => Self::Modification,
            "documentation" => Self::Documentation,
            "defaultLibrary" => Self::DefaultLibrary,
            _ => Self::Unknown(value.to_string()),
        }
    }
}

#[derive(Debug)]
pub struct SemanticTokenModifiers {
    modifiers: Vec<SemanticTokenModifier>,
}

impl SemanticTokenModifiers {
    pub fn new() -> Self {
        Self {
            modifiers: Vec::new(),
        }
    }

    pub fn push(&mut self, modifier: SemanticTokenModifier) -> &mut Self {
        if !self.modifiers.contains(&modifier) {
            self.modifiers.push(modifier);
        }
        self
    }
}
