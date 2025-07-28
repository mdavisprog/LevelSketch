use serde::{
    Deserialize,
    Serialize,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#message
#[derive(Serialize, Deserialize)]
pub struct Message {
    #[serde(default)]
    pub jsonrpc: String,
}

impl Default for Message {
    fn default() -> Self {
        Self {
            jsonrpc: format!("2.0")
        }
    }
}
