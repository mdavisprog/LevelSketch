use async_process::{
    Child,
    Command,
    Stdio,
};
use crate::{
    constructs::{
        DataType,
        DataTypeDB,
        self,
        SymbolTable,
    },
    protocol::{
        document::SymbolKind,
        lifecycle::ServerCapabilities,
        structures::{
            DocumentUri,
            make_file_uri,
            Range,
        },
    },
};
use std::{
    collections::HashMap,
    path::Path,
    sync::{
        Arc,
        mpsc::{
            Receiver,
            self,
            Sender,
            TryRecvError,
        },
        Mutex,
    },
    thread::{
        JoinHandle,
        self,
    },
};
use super::{
    document::Document,
    errors::LanguageServerError,
    handler::MessageHandlerMessage,
    messages::Messages,
    pipes::{
        ReadPipe,
        WritePipe,
    },
    service::LSPServiceOptions,
};

/// Represents a child process and the thread for communication.
pub struct LanguageServer {
    program: String,
    name: String,
    join_handle: Option<JoinHandle<()>>,
    messages: Option<Sender<LanguageServerMessage>>,
    events: Option<Arc<Mutex<Receiver<LanguageServerRunnerEvent>>>>,
    workspace_folder: String,
}

impl LanguageServer {
    pub fn new() -> Self {
        Self {
            program: String::new(),
            name: String::new(),
            join_handle: None,
            messages: None,
            events: None,
            workspace_folder: String::new(),
        }
    }

    pub fn set_name(mut self, name: String) -> Self {
        self.name = name;
        self
    }

    pub fn set_program(mut self, program: String) -> Self {
        self.program = program;
        self
    }

    pub fn set_workspace_folder(mut self, workspace_folder: String) -> Self {
        self.workspace_folder = workspace_folder;
        self
    }

    pub fn run(
        &mut self,
        options: LSPServiceOptions,
    ) -> Result<(), LanguageServerError> {
        // Data to be moved into thread.
        let (sender, receiver) = mpsc::channel::<LanguageServerMessage>();
        let (events_sender, events_receiver) = mpsc::channel::<LanguageServerRunnerEvent>();
        let program = self.program.clone();
        let workspace_folder = self.workspace_folder.clone();

        let join_handle = match thread::Builder::new()
            .name(format!("Language Server: {}", self.name))
            .spawn(move || {
            let mut runner = match LanguageServerRunner::spawn(
                program,
                receiver,
                events_sender,
                options,
            ) {
                Ok(result) => result,
                Err(error) => {
                    println!("{error}");
                    return;
                }
            };

            runner.run(workspace_folder);
        }) {
            Ok(result) => result,
            Err(error) => {
                println!("Failed to spawn thread: {error:?}.");
                return Err(LanguageServerError::FailedToSpawn);
            },
        };

        self.join_handle = Some(join_handle);
        self.messages = Some(sender);
        self.events = Some(Arc::new(Mutex::new(events_receiver)));

        Ok(())
    }

    pub fn stop(&mut self) -> Result<(), LanguageServerError> {
        let Some(handle) = self.join_handle.take() else {
            return Err(LanguageServerError::DidNotState);
        };

        let Some(messages) = self.messages.take() else {
            return Err(LanguageServerError::DidNotState);
        };

        if let Err(_) = messages.send(LanguageServerMessage::Shutdown) {
            return Err(LanguageServerError::FailedToSendMessage);
        }

        if let Err(_) = handle.join() {
            return  Err(LanguageServerError::FailedToStop);
        }

        Ok(())
    }

    pub fn send_message(
        &mut self,
        message: LanguageServerMessage
    ) -> Result<(), LanguageServerError> {
        let Some(messages) = &self.messages else {
            return Err(LanguageServerError::FailedToSendMessage);
        };

        match messages.send(message) {
            Ok(result) => Ok(result),
            Err(_) => Err(LanguageServerError::FailedToSendMessage),
        }
    }

    pub fn poll(&self) -> Option<LanguageServerEvent> {
        let Some(resource) = &self.events else {
            return None;
        };

        let Ok(events) = resource.try_lock() else {
            return None;
        };

        let event = match events.try_recv() {
            Ok(result) => result,
            Err(error) => {
                match error {
                    TryRecvError::Disconnected => {
                        println!("Language server events has been disconnected.");
                    },
                    TryRecvError::Empty => {},
                }

                return None;
            }
        };

        let result = match event {
            LanguageServerRunnerEvent::Initialized => LanguageServerEvent::Initialized,
            LanguageServerRunnerEvent::RetrievedSymbols(symbols) =>
                LanguageServerEvent::RetrievedSymbols(symbols),
        };

        Some(result)
    }

    pub fn name(&self) -> &str {
        &self.name
    }
}

struct LanguageServerRunner {
    process: Child,
    messages: Messages,
    server_messages: Receiver<LanguageServerMessage>,
    events: Sender<LanguageServerRunnerEvent>,
    options: LSPServiceOptions,
    server_capabilities: Option<ServerCapabilities>,
    symbol_requests: HashMap<String, SymbolRequest>,
}

impl LanguageServerRunner {
    fn spawn(
        program: String,
        server_messages: Receiver<LanguageServerMessage>,
        events: Sender<LanguageServerRunnerEvent>,
        options: LSPServiceOptions,
    ) -> Result<Self, String> {
        let process = match Command::new(program)
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .stderr(Stdio::piped())
            .spawn() {
            Ok(child) => child,
            Err(_) => {
                return Err(format!("Failed to spawn child process!"));
            }
        };

        Ok(Self {
            process,
            messages: Messages::new(),
            server_messages: server_messages,
            events: events,
            options,
            server_capabilities: None,
            symbol_requests: HashMap::new(),
        })
    }

    fn run(&mut self, workspace_folder: String) {
        let stdout = self.process
            .stdout
            .take()
            .unwrap_or_else(|| panic!("Failed to retrieve the language server's stdout pipe!"));

        let stderr = self.process
            .stderr
            .take()
            .unwrap_or_else(|| panic!("Failed to retrieve the langauge server's stderr pipe!"));

        let stdin = self.process
            .stdin
            .take()
            .unwrap_or_else(|| panic!("Failed to retrieve the language server's stdin pipe!"));

        let mut out_pipe = ReadPipe::new(stdout);
        let mut error_pipe = ReadPipe::new(stderr);
        let mut write_pipe = WritePipe::new(stdin);

        if let Err(error) = self.messages.initialize(&workspace_folder) {
            panic!("Failed to create 'initialize' request: {error}.");
        }

        loop {
            // Handle any messages from the owning thread.
            if !self.handle_server_messages() {
                break;
            }

            self.messages.handler().send_message(&mut write_pipe);

            write_pipe.poll(self.options.print_send);

            if let Some(buffer) = error_pipe.read() {
                if self.options.print_stderr {
                    print!("{buffer}");
                }
            }

            if let Some(buffer) = out_pipe.read() {
                if self.options.print_stdout {
                    println!("buffer: {buffer}");
                }

                self.messages.handler().append_response(buffer);
            }

            self.messages.handler().process_responses();

            while let Some(message) = self.messages.handler().pop_message() {
                self.handle_response(message);
            }
        }
    }

    fn handle_server_messages(
        &mut self,
    ) -> bool {
        let message = match self.server_messages.try_recv() {
            Ok(result) => result,
            Err(error) => {
                match error {
                    TryRecvError::Disconnected => {
                        println!("Receiver channel disconnected.");
                        return false;
                    },
                    TryRecvError::Empty => {
                        return true;
                    }
                }
            }
        };

        match message {
            LanguageServerMessage::Shutdown => {
                return false;
            },
            LanguageServerMessage::RequestTypes(paths) => {
                for path in paths {
                    if let Err(error) = self.messages.did_open(&path) {
                        println!("Failed to request to open document: {error}.");
                        continue;
                    }

                    if let Err(error) = self.messages.document_symbol(&path) {
                        println!("Failed to request symbols: {error}.");
                        continue;
                    }

                    let Some(request) = SymbolRequest::make(&path) else {
                        continue;
                    };

                    self.symbol_requests.insert(request.uri.clone(), request);
                }
            },
        }

        true
    }

    fn handle_response(&mut self, message: MessageHandlerMessage) {
        match message {
            MessageHandlerMessage::Initialized(result) => {
                self.server_capabilities = if let Some(result) = result {
                    Some(result.capabilities)
                } else {
                    None
                };

                let _ = self.events.send(LanguageServerRunnerEvent::Initialized);
            },
            MessageHandlerMessage::DocumentSymbols(result) => {
                let Some(symbols) = result.data else {
                    return;
                };

                let Some(request) = self.symbol_requests.get_mut(&result.uri) else {
                    println!("DocumentSymbols: Failed to find symbol table for document '{}'.", result.uri);
                    return;
                };

                // Convert and insert all symbols into the request's symbol table. Keep a mapping
                // of a symbol name and its location.
                let db = DataTypeDB::default();
                let mut locations = Vec::<(String, Range)>::new();
                for symbol in &symbols {
                    locations.push((symbol.name.clone(), symbol.location.range));

                    request
                        .symbols
                        .insert(symbol.name.clone(), symbol.clone().into());

                    let data_type = match symbol.kind {
                        SymbolKind::Class => DataType::Object,
                        SymbolKind::Field => {
                            let Some(contents) = request
                                .document
                                .get_contents_from_range(symbol.location.range) else {
                                continue;
                            };

                            let ext = if let Some(ext) = Path::new(&symbol.location.uri).extension() {
                                ext.to_str().unwrap_or("").to_string()
                            } else {
                                String::new()
                            };

                            let tokens: Vec<&str> = contents.split(" ").collect();
                            let mut result = DataType::None;
                            for token in tokens {
                                result = db.get_data_type(&ext, &token.to_string());

                                if result != DataType::None {
                                    break;
                                }
                            }

                            result
                        },
                        _ => DataType::None,
                    };

                    if let Some(symbol_result) = request.symbols.get_mut(&symbol.name) {
                        symbol_result.data_type = data_type;
                    }
                }

                // Move symbols into their respective parent based on the location within
                // the document.
                for (name, location) in locations {
                    for info in &symbols {
                        if !info.location.range.contains(location) {
                            continue;
                        }

                        let Some(symbol) = request.symbols.remove(&name) else {
                            continue;
                        };

                        if let Some(parent) = request.symbols.get_mut(&info.name) {
                            parent.add(symbol);
                        } else {
                            // If the parent wasn't found, then put the symbol back.
                            request.symbols.insert(name, symbol);
                        }

                        break;
                    }
                }

                if let Err(error) = self.messages.semantic_tokens(&request.uri) {
                    println!("Failed to request semantic tokens for document {}: {}",
                        request.uri,
                        error,
                    );
                }
            },
            MessageHandlerMessage::SemanticTokens(result) => {
                let Some(semantic_tokens) = result.data else {
                    return;
                };

                let Some(capabilities) = &self.server_capabilities else {
                    return;
                };

                let Some(provider) = &capabilities.semantic_tokens_provider else {
                    return;
                };

                let Some(mut request) = self.symbol_requests.remove(&result.uri) else {
                    return;
                };

                let Some(tokens) = semantic_tokens.parse_data() else {
                    return;
                };

                let resolved = request.document.resolve(&tokens);
                for item in resolved {
                    let Some(token_type) = provider.legend.token_types.get(item.token_type) else {
                        continue;
                    };

                    let Some(symbol) = request.symbols.get_mut(&item.name) else {
                        continue;
                    };

                    symbol.semantic_type = token_type.as_str().into();

                    for i in 0..u64::BITS {
                        if item.token_modifiers & (1 << i) != 0 {
                            if let Some(modifier) = provider.legend.token_modifiers.get(i as usize) {
                                symbol.modifiers.push(modifier.as_str().into());
                            }
                        }
                    }
                }

                let _ = self.events.send(LanguageServerRunnerEvent::RetrievedSymbols(request.symbols));
            },
        }
    }
}

/// LanguageServer -> LanguageServerRunnable
#[derive(Clone)]
pub enum LanguageServerMessage {
    Shutdown,
    RequestTypes(Vec<String>),
}

/// LanguageServer -> LSPService -> User
pub enum LanguageServerEvent {
    Initialized,
    RetrievedSymbols(SymbolTable),
}

/// LanguageServerRunner -> LanguageServer
enum LanguageServerRunnerEvent {
    Initialized,
    RetrievedSymbols(SymbolTable),
}

struct SymbolRequest {
    uri: DocumentUri,
    document: Document,
    symbols: constructs::SymbolTable,
}

impl SymbolRequest {
    pub fn make(path: &str) -> Option<Self> {
        let Ok(document) = Document::open(path) else {
            return None;
        };

        Some(Self {
            uri: make_file_uri(path),
            document,
            symbols: constructs::SymbolTable::new(),
        })
    }
}
