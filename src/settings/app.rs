use bevy::platform::collections::HashMap;
use serde::{
    Deserialize,
    Serialize,
};

pub type LanguageServerMap = HashMap<String, String>;

#[derive(Serialize, Deserialize, Debug)]
pub struct LSPSettings {
    pub servers: LanguageServerMap,
}

impl Default for LSPSettings {
    fn default() -> Self {
        Self {
            servers: HashMap::from([
                (format!("cpp"), format!("clangd")),
                (format!("rs"), format!("rust-analyzer")),
            ]),
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
pub struct AppSettings {
    pub lsp: LSPSettings,
}

impl Default for AppSettings {
    fn default() -> Self {
        Self {
            lsp: LSPSettings::default(),
        }
    }
}
