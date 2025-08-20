use crate::protocol::structures::{
    TextDocumentIdentifier,
    TextDocumentItem,
};
use serde::{
    Deserialize,
    Serialize,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#didOpenTextDocumentParams
#[derive(Serialize, Deserialize)]
pub struct DidOpenTextDocumentParams {
    // The document that was opened.
    #[serde(rename = "textDocument")]
    pub text_document: TextDocumentItem,
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#didCloseTextDocumentParams
#[derive(Serialize, Deserialize)]
pub struct DidCloseTextDocumentParams {
    // The document that was closed.
    #[serde(rename = "textDocument")]
    pub text_document: TextDocumentIdentifier,
}
