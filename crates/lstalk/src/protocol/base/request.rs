use serde::{
    de::DeserializeOwned,
    Deserialize,
    Serialize,
};
use super::{
    message::{
        Messagable,
        Message,
    },
    types::*,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#requestMessage
#[derive(Serialize, Deserialize)]
pub struct Request {
    #[serde(flatten)]
    pub message: Message,

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

    pub fn parse_params<T: DeserializeOwned>(&self) -> Option<T> {
        let Some(params) = &self.params else {
            return None;
        };

        let json = match serde_json::to_string(&params) {
            Ok(json) => json,
            Err(error) => {
                println!("Response_parse_result: Failed to convert '{params}' into string: {error:?}");
                return None;
            }
        };

        let result = match serde_json::from_str::<T>(&json) {
            Ok(result) => result,
            Err(error) => {
                println!("Response::parse_result: Failed to parse result of type {}: {error:?}", std::any::type_name::<T>());
                return None;
            }
        };

        Some(result)
    }
}

impl Messagable for Request {}
