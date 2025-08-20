use crate::protocol::{
    document::{
        DidCloseTextDocumentParams,
        DidOpenTextDocumentParams,
        DocumentSymbolParams,
        SemanticTokens,
        SemanticTokensParams,
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
    DocumentResponse,
    MessageHandler,
    MessageHandlerMessage,
    MessageHandlerError,
    MessageResponse,
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
        let result = self.handler.queue_request(
            "textDocument/documentSymbol",
            DocumentSymbolParams::new(path),
            on_document_symbol,
        );

        let _ = self.handler.queue_request(
            "textDocument/semanticTokens/full",
            SemanticTokensParams::new(path),
            on_semantic_tokens,
        );

        result
    }

    pub fn handler(&mut self) -> &mut MessageHandler {
        &mut self.handler
    }
}

fn on_initialize_response(
    message_response: &mut MessageResponse,
) {
    let parsed = message_response.response.parse_result::<InitializeResult>();
    if let Some(result) = &parsed {
        if let Some(server_info) = &result.server_info {
            println!("Successfully connected to language server {}.", server_info.name);
            println!("Version: {}", if let Some(version) = &server_info.version {
                version
            } else {
                ""
            });
        }
    }

    let _ = message_response
        .messages
        .queue_notification("initialized", InitializedParams);

    message_response
        .messages
        .push_message(MessageHandlerMessage::Initialized(parsed));
}

fn on_document_symbol(message_response: &mut MessageResponse) {
    let Some(symbols) = message_response.response.parse_result::<Vec<SymbolInformation>>() else {
        println!("documentSymbol did not return SymbolInformation! Need to implement DocumentSymbol");
        return;
    };

    let Some(params) = message_response.request.parse_params::<DocumentSymbolParams>() else {
        println!("Failed to retrieve params made for documentSymbol request!");
        return;
    };

    let response = DocumentResponse {
        uri: params.text_document.uri,
        data: Some(symbols),
    };

    message_response
        .messages
        .push_message(MessageHandlerMessage::DocumentSymbols(response));
}

fn on_semantic_tokens(message_response: &mut MessageResponse) {
    let Some(params) = message_response.request.parse_params::<SemanticTokensParams>() else {
        println!("Failed to retrieve params made for semanticTokens request!");
        return;
    };

    let semantic_tokens = message_response.response.parse_result::<SemanticTokens>();

    let response = DocumentResponse {
        uri: params.text_document.uri.clone(),
        data: semantic_tokens,
    };

    let _ = message_response
        .messages
        .push_message(MessageHandlerMessage::SemanticTokens(response))
        .queue_notification(
            "textDocument/didClose",
            DidCloseTextDocumentParams {
                text_document: params.text_document,
            },
        );
}
