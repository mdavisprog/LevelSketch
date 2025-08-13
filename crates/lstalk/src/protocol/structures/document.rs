use crate::protocol::base::types::*;
use serde::{
    Deserialize,
    Serialize,
};
use std::{
    fs::File,
    io::{
        self,
        Read
    },
    path::Path,
};
use super::{
    DocumentUri,
    make_file_uri,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentIdentifier
#[derive(Serialize, Deserialize, Default)]
pub struct TextDocumentIdentifier {
    // The text document's URI.
    pub uri: DocumentUri,
}

impl TextDocumentIdentifier {
    pub fn new(uri: DocumentUri) -> Self {
        Self {
            uri,
        }
    }

    pub fn make(path: &str) -> Self {
        Self {
            uri: make_file_uri(path),
        }
    }
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentItem
#[derive(Serialize, Deserialize)]
pub struct TextDocumentItem {
    // The text document's URI.
    pub uri: DocumentUri,

    // The text document's language identifier.
    #[serde(rename = "languageId")]
    pub language_id: String,

    // The version number of this document (it will increase after each
    // change, including undo/redo).
    pub version: Integer,

    // The content of the opened text document.
    pub text: String,
}

impl TextDocumentItem {
    pub fn new(uri: &str) -> Result<Self, io::Error> {
        let (language_id, text) = {
            let path = Path::new(&uri);
            let extension = match path.extension() {
                Some(result) => result.to_str().unwrap_or_default().to_string(),
                None => {
                    let error = format!("Given URI {uri} does not have an extension.");
                    return Err(io::Error::new(io::ErrorKind::InvalidFilename, error));
                },
            };

            let mut contents = String::new();
            let mut file = File::open(path)?;
            file.read_to_string(&mut contents)?;

            (extension, contents)
        };

        Ok(Self {
            uri: make_file_uri(uri),
            language_id,
            version: 1,
            text,
        })
    }
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#position
#[derive(Serialize, Deserialize, Default, Clone, Copy)]
pub struct Position {
    // Line position in a document (zero-based).
    pub line: UInteger,

    // Character offset on a line in a document (zero-based). The meaning of this
    // offset is determined by the negotiated `PositionEncodingKind`.
    //
    // If the character value is greater than the line length it defaults back
    // to the line length.
    pub character: UInteger,
}

impl std::fmt::Display for Position {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "line: {} character: {}", self.line, self.character)
    }
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#range
#[derive(Serialize, Deserialize, Default)]
pub struct Range {
    // The range's start position.
    pub start: Position,

    // The range's end position.
    pub end: Position,
}

impl std::fmt::Display for Range {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "start: {} end: {}", self.start, self.end)
    }
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location
///
/// Represents a location inside a resource, such as a line inside a text file.
#[derive(Serialize, Deserialize, Default)]
pub struct Location {
    pub uri: DocumentUri,
    pub range: Range,
}

impl std::fmt::Display for Location {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "uri: {} range: {}", self.uri, self.range)
    }
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams
///
/// A parameter literal used in requests to pass a text document and a position inside that document.
/// It is up to the client to decide how a selection is converted into a position when issuing a
/// request for a text document. The client can for example honor or ignore the selection direction
/// to make LSP request consistent with features implemented internally.
#[derive(Serialize, Deserialize, Default)]
pub struct TextDocumentPositionParams {
    // The text document.
    #[serde(rename = "textDocument")]
    pub text_document: TextDocumentIdentifier,

    // The position inside the text document.
    pub position: Position,
}
