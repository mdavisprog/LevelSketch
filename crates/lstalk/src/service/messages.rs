use crate::protocol::{
    base::Response,
    document::{
        DidOpenTextDocumentParams,
        DocumentSymbolParams,
        SymbolInformation,
    },
    lifecycle::{
        InitializedParams,
        InitializeParams,
        InitializeResult,
    },
    structures::TextDocumentItem,
};
use super::handler::{
    MessageHandler,
    MessageHandlerMessage,
    MessageHandlerError,
};

pub struct Messages {
    handler: MessageHandler
}

impl Messages {
    pub fn new() -> Self {
        Self {
            handler: MessageHandler::new(),
        }
    }

    pub fn initialize(&mut self, workspace_folder: &str) -> Result<(), MessageHandlerError> {
        self.handler.queue_request(
            "initialize",
            InitializeParams::new_with_workspace(workspace_folder),
            on_initialize_response,
        )
    }

    pub fn did_open(&mut self, path: &str) -> Result<(), MessageHandlerError> {
        let text_document = match TextDocumentItem::new(path) {
            Ok(result) => result,
            Err(_) => {
                return Err(MessageHandlerError::FailedToInitializeResource);
            }
        };

        self.handler.queue_notification(
            "textDocument/didOpen",
            DidOpenTextDocumentParams {
                text_document,
            },
        )
    }

    pub fn document_symbol(&mut self, path: &str) -> Result<(), MessageHandlerError> {
        self.handler.queue_request(
            "textDocument/documentSymbol",
            DocumentSymbolParams::new(path),
            on_document_symbol,
        )
    }

    pub fn handler(&mut self) -> &mut MessageHandler {
        &mut self.handler
    }
}

fn on_initialize_response(
    messages: &mut MessageHandler,
    response: &Response,
) {
    if let Some(result) = response.parse_result::<InitializeResult>() {
        let server_info = result.server_info.unwrap_or_default();
        println!("Successfully connected to language server {}.", server_info.name);
        println!("Version: {}", server_info.version.unwrap_or(format!("undefined")));
    }
    let _ = messages.queue_notification("initialized", InitializedParams);
    messages.push_message(MessageHandlerMessage::Initialized);
}

fn on_document_symbol(
    _messages: &mut MessageHandler,
    response: &Response,
) {
    let Some(symbols) = response.parse_result::<Vec<SymbolInformation>>() else {
        println!("documentSymbol did not return SymbolInformation! Need to implement DocumentSymbol");
        return;
    };

    println!("Retrieved {} symbols.", symbols.len());
}
