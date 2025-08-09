use serde::{
	de::DeserializeOwned,
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

impl Response {
	pub fn parse_result<T: DeserializeOwned>(&self) -> Option<T> {
		let Some(object) = &self.result else {
			return None;
		};

		let json = match serde_json::to_string(&object) {
			Ok(json) => json,
			Err(error) => {
				println!("Response_parse_result: Failed to convert '{object}' into string!");
				return None;
			}
		};

		let result = match serde_json::from_str::<T>(&json) {
			Ok(result) => result,
			Err(error) => {
				println!("Response::parse_result: Failed to parse result of type {}.", std::any::type_name::<T>());
				return None;
			}
		};

		Some(result)
	}
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
