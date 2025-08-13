use serde::{
    Deserialize,
    Serialize,
};
use super::{
    notification::Notification,
    request::Request,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#message
#[derive(Serialize, Deserialize)]
pub struct Message {
    #[serde(default)]
    pub jsonrpc: String,
}

impl Default for Message {
    fn default() -> Self {
        Self {
            jsonrpc: format!("2.0")
        }
    }
}

pub trait Messagable: Serialize {
    fn encode(&self) -> Result<String, String> {
        let payload = match serde_json::to_string(self) {
            Ok(result) => result,
            Err(error) => {
                return Err(format!("{error:?}"));
            }
        };

        Ok(format!("Content-Length: {}\r\n\r\n{payload}", payload.len()))
    }
}

pub enum MessageType {
    Notification(Notification),
    Request(Request),
}
