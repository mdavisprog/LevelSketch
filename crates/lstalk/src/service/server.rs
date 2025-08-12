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
            TryRecvError,
        },
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
    service::{
        LSPServiceMessage,
        LSPServiceOptions,
    },
};

/// Represents a child process and the thread for communication.
pub(super) struct LanguageServer {
    process: Child,
    messages: Messages,
    options: Arc<LSPServiceOptions>,
    is_initialized: bool,
}

impl LanguageServer {
    pub fn spawn(program: &str, options: Arc<LSPServiceOptions>) -> Result<Self, LanguageServerError> {
        let process = match Command::new(program)
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .stderr(Stdio::piped())
            .spawn() {
            Ok(child) => child,
            Err(_) => {
                return Err(LanguageServerError::FailedToSpawn);
            }
        };

        Ok(Self {
            process,
            messages: Messages::new(),
            options,
            is_initialized: false,
        })
    }

    pub fn run(&mut self, receiver: Receiver<LSPServiceMessage>) {
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

        let Ok(payload) = self.messages.initialize() else {
            panic!("Failed to create 'initialize' request!");
        };

        write_pipe.write(payload);

        loop {
            // Handle any messages from the owning thread.
            match receiver.try_recv() {
                Ok(message) => {
                    match message {
                        LSPServiceMessage::Shutdown => {
                            break;
                        },
                        LSPServiceMessage::RequestTypes(paths) => {
                            for path in paths {
                                match self.messages.did_open(&path) {
                                    Ok(payload) => {
                                        write_pipe.write(payload);

                                        match self.messages.document_symbol(&path) {
                                            Ok(payload) => {
                                                write_pipe.write(payload);
                                            },
                                            Err(error) => {
                                                println!("Failed to request symbols: {error}.");
                                            },
                                        }
                                    },
                                    Err(error) => {
                                        println!("Failed to request to open document: {error}");
                                    },
                                };
                            }
                        },
                    }
                },
                Err(error) => {
                    match error {
                        TryRecvError::Disconnected => {
                            println!("Service thread received a disconnected error.");
                            break;
                        },
                        TryRecvError::Empty => {},
                    }
                },
            }

            write_pipe.poll();

            if let Some(buffer) = error_pipe.read() {
                if self.options.print_stderr {
                    print!("{buffer}");
                }
            }

            if let Some(buffer) = out_pipe.read() {
                if self.options.print_stdout {
                    println!("buffer: {buffer}");
                }

                self.messages.handler().handle_response(buffer);
            }

            while let Some(message) = self.messages.handler().pop_message() {
                match message {
                    MessageHandlerMessage::Initialized => {
                        self.is_initialized = true;
                    },
                }
            }
        }
    }
}
