use async_process::{
    Child,
    Command,
    Stdio,
};
use std::{
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
                match message {
                    MessageHandlerMessage::Initialized => {
                        let _ = self.events.send(LanguageServerRunnerEvent::Initialized);
                    },
                }
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
                    }
                }
            },
        }

        true
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
}

/// LanguageServerRunner -> LanguageServer
enum LanguageServerRunnerEvent {
    Initialized,
}
