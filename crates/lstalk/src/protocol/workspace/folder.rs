use crate::protocol::structures::{
    make_file_uri,
    URI,
};
use serde::{
    Deserialize,
    Serialize,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workspaceFolder
#[derive(Serialize, Deserialize)]
pub struct WorkspaceFolder {
	// The associated URI for this workspace folder.
	pub uri: URI,

	// The name of the workspace folder. Used to refer to this
	// workspace folder in the user interface.
	pub name: String,
}

impl WorkspaceFolder {
    pub fn new(path: &str, name: String) -> Self {
        Self {
            uri: make_file_uri(path),
            name,
        }
    }
}
