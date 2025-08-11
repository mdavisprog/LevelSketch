use serde::{
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

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#notificationMessage
#[derive(Serialize, Deserialize)]
pub struct Notification {
    #[serde(flatten)]
    pub message: Message,

	// The method to be invoked.
	pub method: String,

	// The method's params.
    // LSPArray | LSPObject
	pub params: Option<LSPAny>,
}

impl Notification {
    pub fn new(method: &str, params: LSPAny) -> Self {
        Self {
            message: Message::default(),
            method: method.to_string(),
            params: Some(params),
        }
    }
}

impl Messagable for Notification {}
