use serde::{
    Deserialize,
    Serialize,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#progress
#[derive(Serialize, Deserialize)]
pub enum ProgressToken {
    Int(i32),
    String(String),
}

impl Default for ProgressToken {
    fn default() -> Self {
        Self::Int(0)
    }
}
