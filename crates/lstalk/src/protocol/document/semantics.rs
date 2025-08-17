use crate::protocol::{
    base::types::*,
    structures::{
        DocumentUri,
        PartialResultParams,
        Range,
        TextDocumentIdentifier,
        WorkDoneProgressParams,
    },
};
use serde::{
    Deserialize,
    Serialize,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensParams
#[derive(Serialize, Deserialize)]
pub struct SemanticTokensParams {
    #[serde(flatten)]
    pub partial_result: PartialResultParams,

    #[serde(flatten)]
    pub work_done_progress: WorkDoneProgressParams,

	//  The text document.
    #[serde(rename = "textDocument")]
    pub text_document: TextDocumentIdentifier,
}

impl SemanticTokensParams {
    pub fn new(path: &str) -> Self {
        Self {
            partial_result: PartialResultParams::default(),
            work_done_progress: WorkDoneProgressParams::default(),
            text_document: TextDocumentIdentifier::make(path),
        }
    }
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokens
#[derive(Serialize, Deserialize)]
pub struct SemanticTokens {
    // An optional result id. If provided and clients support delta updating
    // the client will include the result id in the next semantic token request.
    // A server can then instead of computing all semantic tokens again simply
    // send a delta.
    #[serde(rename = "resultId")]
	pub result_id: Option<String>,

    // The actual tokens.
    pub data: Vec<UInteger>,
}

impl SemanticTokens {
    pub fn parse_data(&self) -> Option<Vec<SemanticToken>> {
        if self.data.len() < 5 {
            return None;
        }

        let mut result = Vec::<SemanticToken>::new();
        let mut index = 0;
        while index < self.data.len() - 5 {
            let data = &self.data[index..(index+5)];

            let token = SemanticToken {
                line: data[0],
                start: data[1],
                length: data[2],
                token_type: data[3],
                modifiers: data[4],
            };

            result.push(token);

            index += 5;
        }

        Some(result)
    }
}

/// Representation of a single token representation within the data array.
pub struct SemanticToken {
    pub line: UInteger,
    pub start: UInteger,
    pub length: UInteger,
    pub token_type: UInteger,
    pub modifiers: UInteger,
}
