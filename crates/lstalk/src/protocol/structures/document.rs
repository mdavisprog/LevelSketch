use crate::protocol::base::types::*;
use serde::{
    Deserialize,
    Serialize,
};
use std::{
	fs::File, io::{self, Read}, path::Path
};
use super::{
	DocumentUri,
	make_file_uri,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentIdentifier
#[derive(Serialize, Deserialize)]
pub struct TextDocumentIdentifier {
	// The text document's URI.
	pub uri: DocumentUri,
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
	pub fn new(uri: String) -> Result<Self, io::Error> {
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
			uri: make_file_uri(&uri),
			language_id,
			version: 1,
			text,
		})
	}
}
