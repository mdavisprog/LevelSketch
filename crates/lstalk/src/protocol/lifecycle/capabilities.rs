use crate::protocol::base::types::*;
use serde::{
    Deserialize,
    Serialize,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#serverCapabilities
#[derive(Serialize, Deserialize)]
pub struct ServerCapabilities {
    // The server provides semantic tokens support.
    //
    // @since 3.16.0
    // TODO: Support SemanticTokensRegistrationOptions
    #[serde(rename = "semanticTokensProvider")]
    pub semantic_tokens_provider: Option<SemanticTokensOptions>,
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensLegend
#[derive(Serialize, Deserialize)]
pub struct SemanticTokensLegend {
    // The token types a server uses.
    #[serde(rename = "tokenTypes")]
    pub token_types: Vec<String>,

    // The token modifiers a server uses.
    #[serde(rename = "tokenModifiers")]
    pub token_modifiers: Vec<String>,
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensOptions
#[derive(Serialize, Deserialize)]
pub struct SemanticTokensOptions {
    // The legend used by the server
    pub legend: SemanticTokensLegend,

    // Server supports providing semantic tokens for a specific range
    // of a document.
    //
    // boolean | {
    // };
    pub range: Option<LSPAny>,

    // Server supports providing semantic tokens for a full document.
    //
    // boolean | {
    //	 /**
    //	  * The server supports deltas for full documents.
    //	  */
    //	 delta?: boolean;
    // };
    pub full: Option<LSPAny>,
}
