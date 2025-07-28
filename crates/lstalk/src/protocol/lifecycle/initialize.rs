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

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initialized
#[derive(Serialize, Deserialize)]
pub struct InitializedParams;
