use crate::protocol::base::{
    Messagable,
    Notification,
    Request,
    Response,
    types::*,
};
use serde::Serialize;
use std::rc::Rc;

pub struct MessageHandler {
    id: i64,
    requests: Vec<RequestItem>,
    response_buffer: String,
    messages: Vec<MessageHandlerMessage>,
}

impl MessageHandler {
    pub fn new() -> Self {
        Self {
            id: 1,
            requests: Vec::new(),
            response_buffer: String::new(),
            messages: Vec::with_capacity(8),
        }
    }

    pub fn make_request<T: Serialize>(
        &mut self,
        method: &str,
        params: T,
        callback: impl Fn(&mut Self, &Response) + 'static,
    ) -> Result<String, MessageHandlerError> {
        Self::make_request_internal(self, method, params, Some(callback))
    }

    pub fn make_request_forget<T: Serialize>(
        &mut self,
        method: &str,
        params: T,
    ) -> Result<String, MessageHandlerError> {
        Self::make_request_internal(self, method, params, None::<&dyn Fn(&mut Self, &Response)>)
    }

    pub fn make_notification<T: Serialize>(
        &mut self,
        method: &str,
        params: T
    ) -> Result<String, MessageHandlerError> {
        let value = Self::convert(params)?;
        let notification = Notification::new(method, value.into());
        let payload = Self::encode(&notification)?;
        Ok(payload)
    }

    pub fn push_message(&mut self, message: MessageHandlerMessage) -> &mut Self {
        self.messages.push(message);
        self
    }

    pub fn pop_message(&mut self) -> Option<MessageHandlerMessage> {
        self.messages.pop()
    }

    pub fn handle_response(&mut self, response: String) {
        self.response_buffer += &response;

        const CONTENT_LENGTH: &str = "Content-Length: ";
        // Check to find the 'Content-Length' token that marks the start of a response.
        let Some(index) = self.response_buffer.find(CONTENT_LENGTH) else {
            println!("Failed to find {CONTENT_LENGTH}");
            return;
        };

        // Define the start and end index for the length.
        let start = index + CONTENT_LENGTH.len();
        let end = match self.response_buffer[start..].find("\r\n") {
            Some(index) => start + index,
            None => {
                println!("Failed to find \\r\\n characters");
                return;
            },
        };

        // Get the token slice and attempt to parse.
        let length_token = &self.response_buffer[start..end];
        let length = match length_token.parse::<usize>() {
            Ok(result) => result,
            Err(error) => {
                println!("Failed to parse {}: {error:?}", length_token);
                return;
            },
        };

        // Define the content start and end indices and ensure all data has arrived.
        let content_start = end + "\r\n\r\n".len();
        let content_end = content_start + length;
        let remaining = self.response_buffer.len() - content_start;
        if remaining < length {
            println!("Not enough data collected for response. Waiting for more...");
            return;
        }

        // Deserialize content into a Response object. The contents of this response
        // is drained from the response buffer.
        let contents = &self.response_buffer[content_start..content_end];
        let response = match serde_json::from_str::<Response>(contents) {
            Ok(response) => response,
            Err(error) => {
                println!("Failed to deserialize contents: {error:?}.");
                self.response_buffer.drain(0..content_end);
                return;
            }
        };

        self.response_buffer.drain(0..content_end);

        // Get the optional callback from the request item based on the response id.
        let callback = {
            let mut result: Option<Rc<dyn Fn(&mut Self, &Response)>> = None;

            for item in &self.requests {
                if let Some(id) = &response.id {
                    if item.request.id == *id {
                        result = item.callback.clone();
                        break;
                    }
                }
            }

            result
        };

        // If a callback was supplied, then invoke.
        if let Some(callback) = callback {
            (*callback)(self, &response);
        }

        // Finally, remove the request from the list.
        self.requests.retain(|item| {
            if let Some(id) = &response.id {
                if item.request.id == *id {
                    false
                } else {
                    true
                }
            } else {
                false
            }
        });
    }

    fn make_request_internal<T: Serialize>(
        &mut self,
        method: &str,
        params: T,
        callback: Option<impl Fn(&mut Self, &Response) + 'static>,
    ) -> Result<String, MessageHandlerError> {
        let value = Self::convert(params)?;

        let request = Request::new(
            LSPAny::Integer(self.id),
            method,
            value.into(),
        );

        let payload = Self::encode(&request)?;

        self.requests.push(RequestItem {
            request,
            callback: if callback.is_some() { Some(Rc::new(callback.unwrap())) } else { None },
        });
        self.id += 1;

        Ok(payload)
    }

    fn convert<T: Serialize>(params: T) -> Result<serde_json::Value, MessageHandlerError> {
        match serde_json::to_value(params) {
            Ok(value) => Ok(value),
            Err(error) => {
                return Err(MessageHandlerError::FailedToParse(format!("{error:?}")));
            }
        }
    }

    fn encode<T: Messagable>(message: &T) -> Result<String, MessageHandlerError> {
        match message.encode() {
            Ok(payload) => Ok(payload),
            Err(error) => {
                return Err(MessageHandlerError::FailedToSerialize(error));
            }
        }
    }
}

pub enum MessageHandlerError {
    FailedToParse(String),
    FailedToSerialize(String),
}

impl std::fmt::Display for MessageHandlerError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::FailedToParse(error) => write!(f, "Failed to parse: {error}"),
            Self::FailedToSerialize(error) => write!(f, "Failed to serialize: {error}"),
        }
    }
}

struct RequestItem {
    request: Request,
    callback: Option<Rc<dyn Fn(&mut MessageHandler, &Response)>>,
}

pub enum MessageHandlerMessage {
    Initialized,
}
