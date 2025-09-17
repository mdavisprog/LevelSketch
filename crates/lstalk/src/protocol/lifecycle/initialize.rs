use crate::protocol::{
    base::types::*,
    structures::{
        DocumentUri,
        make_file_uri,
        WorkDoneProgressParams,
    },
    workspace::WorkspaceFolder,
};
use serde::{
    Deserialize,
    Serialize,
};
use std::path::Path;
use super::capabilities::{
    ClientCapabilities,
    ServerCapabilities,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initializeParams
#[derive(Serialize, Deserialize, Default)]
pub struct InitializeParams {
    #[serde(flatten)]
    work_done_progress: WorkDoneProgressParams,

    // The process Id of the parent process that started the server. Is null if
    // the process has not been started by another process. If the parent
    // process is not alive then the server should exit (see exit notification)
    // its process.
    #[serde(rename = "processId")]
    pub process_id: Option<Integer>,

	// The rootPath of the workspace. Is null
	// if no folder is open.
	//
	// @deprecated in favour of `rootUri`.
    #[serde(rename = "rootPath")]
	pub root_path: Option<String>,

	// The rootUri of the workspace. Is null if no
	// folder is open. If both `rootPath` and `rootUri` are set
	// `rootUri` wins.
	//
	// @deprecated in favour of `workspaceFolders`
    #[serde(rename = "rootUri")]
	pub root_uri: Option<DocumentUri>,

	// The capabilities provided by the client (editor or tool)
	pub capabilities: ClientCapabilities,

	// The workspace folders configured in the client when the server starts.
	// This property is only available if the client supports workspace folders.
	// It can be `null` if the client supports workspace folders but none are
	// configured.
	//
	// @since 3.6.0
    #[serde(rename = "workspaceFolders")]
	pub workspace_folders: Option<Vec<WorkspaceFolder>>,
}

impl InitializeParams {
    pub fn new() -> Self {
        Self {
            work_done_progress: WorkDoneProgressParams::default(),
            process_id: Some(std::process::id() as Integer),
            root_path: None,
            root_uri: None,
            capabilities: ClientCapabilities::default(),
            workspace_folders: None,
        }
    }

    pub fn new_with_workspace(path: &str) -> Self {
        let name = if let Some(file_name) = Path::new(path).file_name() {
            file_name
                .to_str()
                .unwrap_or_else(|| path)
                .to_string()
        } else {
            path.to_string()
        };

        let uri = make_file_uri(path);

        let mut result = Self::new();
        result.root_path = Some(uri.clone());
        result.root_uri = Some(uri);
        result.workspace_folders = Some(vec![
            WorkspaceFolder::new(path, name),
        ]);

        result
    }
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initializeResult
#[derive(Deserialize)]
pub struct InitializeResult {
    // The capabilities the language server provides.
    pub capabilities: ServerCapabilities,

    // Information about the server.
    //
    // @since 3.15.0
    #[serde(rename = "serverInfo")]
    pub server_info: Option<ServerInfo>,
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initialized
#[derive(Serialize, Deserialize)]
pub struct InitializedParams;

#[derive(Deserialize, Default)]
pub struct ServerInfo {
    // The name of the server as defined by the server.
    pub name: String,

    // The server's version as defined by the server.
    pub version: Option<String>,
}
