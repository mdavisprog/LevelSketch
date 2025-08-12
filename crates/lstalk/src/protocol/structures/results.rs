use crate::protocol::base::ProgressToken;
use serde::{
    Deserialize,
    Serialize,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#partialResultParams
#[derive(Serialize, Deserialize, Default)]
pub struct PartialResultParams {
	// An optional token that a server can use to report partial results (e.g.
	// streaming) to the client.
    #[serde(rename = "partialResultToken")]
	pub partial_result_token: Option<ProgressToken>,
}
