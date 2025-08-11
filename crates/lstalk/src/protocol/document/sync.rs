use crate::protocol::structures::TextDocumentItem;
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
