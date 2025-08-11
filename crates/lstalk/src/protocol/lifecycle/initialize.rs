use crate::protocol::{
	base::types::*,
	structures::WorkDoneProgressParams,
};
use serde::{
	Deserialize,
	Serialize,
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
}

impl InitializeParams {
	pub fn new() -> Self {
		Self {
			work_done_progress: WorkDoneProgressParams::default(),
			process_id: Some(std::process::id() as Integer),
		}
	}
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initializeResult
#[derive(Deserialize)]
pub struct InitializeResult {
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
