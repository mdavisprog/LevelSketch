use crate::protocol::{
    base::{
        Messagable,
        MessageType,
        Notification,
        Request,
        Response,
        types::*,
    },
    document::{
        SemanticTokens,
        SymbolInformation,
    },
    lifecycle::InitializeResult,
    structures::DocumentUri,
};
use serde::Serialize;
use std::rc::Rc;
use super::pipes::WritePipe;

pub struct MessageHandler {
    id: i64,
    queued: MessageQueue,
    sent: MessageQueue,
    response_buffer: String,
    messages: Vec<MessageHandlerMessage>,
}

impl MessageHandler {
    pub fn new() -> Self {
        Self {
            id: 1,
            queued: Vec::new(),
            sent: Vec::new(),
            response_buffer: String::new(),
            messages: Vec::with_capacity(8),
        }
    }

    pub fn queue_notification<T: Serialize>(
        &mut self,
        method: &str,
        params: T,
    ) -> Result<(), MessageHandlerError> {
        let value = Self::convert(params)?;
        let notification = Notification::new(
            method,
            value.into(),
        );

        self.queued.insert(0, MessageItem {
            message: MessageType::Notification(notification),
            callback: None,
        });

        Ok(())
    }

    pub fn queue_request<T: Serialize>(
        &mut self,
        method: &str,
        params: T,
        callback: impl Fn(&mut MessageResponse) + 'static,
    ) -> Result<(), MessageHandlerError> {
        let value = Self::convert(params)?;

        let request = Request::new(
            LSPAny::Integer(self.id),
            method,
            value.into(),
        );

        self.queued.insert(0, MessageItem {
            message: MessageType::Request(request),
            callback: Some(Rc::new(callback)),
        });
        self.id += 1;

        Ok(())
    }

    pub fn push_message(&mut self, message: MessageHandlerMessage) -> &mut Self {
        self.messages.push(message);
        self
    }

    pub fn pop_message(&mut self) -> Option<MessageHandlerMessage> {
        self.messages.pop()
    }

    pub fn send_message(&mut self, writer: &mut WritePipe) -> bool {
        let Some(message) = self.queued.pop() else {
            return false;
        };

        let encoded = match &message.message {
            MessageType::Notification(notification) => {
                Self::encode(notification)
            },
            MessageType::Request(request) => {
                let encoded = Self::encode(request);
                self.sent.push(message);
                encoded
            }
        };

        match encoded {
            Ok(payload) => {
                writer.write(payload);
            },
            Err(error) => {
                println!("Failed to encode message: {error}.");
            },
        }

        true
    }

    pub fn append_response(&mut self, response: String) -> &mut Self {
        self.response_buffer += &response;
        self
    }

    pub fn process_responses(&mut self) {
        if self.response_buffer.is_empty() {
            return;
        }

        const CONTENT_LENGTH: &str = "Content-Length:";
        // Check to find the 'Content-Length' token that marks the start of a response.
        let Some(index) = self.response_buffer.find(CONTENT_LENGTH) else {
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
        let length_token = &self.response_buffer[start..end].trim();
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

        let sent = self.sent.extract_if(.., |element| -> bool {
            match &element.message {
                MessageType::Request(request) => {
                    if let Some(id) = &response.id {
                        request.id == *id
                    } else {
                        false
                    }
                },
                _ => false,
            }
        })
        .collect::<Vec<MessageItem>>();

        for item in sent {
            let Some(callback) = item.callback else {
                continue;
            };

            let MessageType::Request(request) = item.message else {
                continue;
            };

            let mut object = MessageResponse {
                messages: self,
                request: &request,
                response: &response,
            };

            (*callback)(&mut object);
        }
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
    FailedToInitializeResource,
}

impl std::fmt::Display for MessageHandlerError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::FailedToParse(error) => write!(f, "Failed to parse: {error}"),
            Self::FailedToSerialize(error) => write!(f, "Failed to serialize: {error}"),
            Self::FailedToInitializeResource => write!(f, "Failed to initialize resource"),
        }
    }
}

pub struct DocumentResponse<T> {
    pub uri: DocumentUri,
    pub data: Option<T>,
}

pub enum MessageHandlerMessage {
    Initialized(Option<InitializeResult>),
    // TODO: Need a Or type with this Or<SymbolInformation, DocumentSymbol>.
    DocumentSymbols(DocumentResponse<Vec<SymbolInformation>>),
    SemanticTokens(DocumentResponse<SemanticTokens>),
}

pub(super) struct MessageResponse<'a> {
    pub messages: &'a mut MessageHandler,
    pub response: &'a Response,
    pub request: &'a Request,
}

struct MessageItem {
    message: MessageType,
    callback: Option<Rc<dyn Fn(&mut MessageResponse)>>,
}

type MessageQueue = Vec<MessageItem>;
