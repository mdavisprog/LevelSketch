use serde::{
    Deserialize,
    Serialize,
};
use super::{
    message::Message,
    types::*,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#requestMessage
#[derive(Serialize, Deserialize)]
pub struct Request {
    #[serde(flatten)]
    message: Message,

	// The request id.
    // number | string
	pub id: LSPAny,

	// The method to be invoked.
	pub method: String,

	// The method's params.
    // LSPArray | LSPObject
	pub params: Option<LSPAny>,
}

impl Request {
    pub fn new(id: LSPAny, method: &str, params: LSPAny) -> Self {
        Self {
            message: Message::default(),
            id,
            method: method.into(),
            params: Some(params),
        }
    }

    pub fn serialize(&self) -> Result<String, String> {
        let payload = match serde_json::to_string(self) {
            Ok(result) => result,
            Err(error) => {
                return Err(format!("{error:?}"));
            }
        };

        Ok(format!("Content-Length: {}\r\n\r\n{payload}", payload.len()))
    }
}
