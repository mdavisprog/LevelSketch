use serde::{
    Deserialize,
    Serialize,
};
use super::{
    message::Message,
    types::*,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#responseMessage
#[derive(Serialize, Deserialize)]
pub struct Response {
    #[serde(flatten)]
    pub message: Message,

	// The request id.
	pub id: LSPAny,

	// The result of a request. This member is REQUIRED on success.
	// This member MUST NOT exist if there was an error invoking the method.
	pub result: Option<LSPAny>,

	// The error object in case a request fails.
	pub error: Option<ResponseError>,
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#responseError
#[derive(Serialize, Deserialize)]
pub struct ResponseError {
	// A number indicating the error type that occurred.
	pub code: Integer,

	// A string providing a short description of the error.
	pub message: String,

	// A primitive or structured value that contains additional
	// information about the error. Can be omitted.
	pub data: Option<LSPAny>,
}
