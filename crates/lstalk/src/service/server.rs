use async_process::{
    Child,
    Command,
    Stdio,
};
use crate::protocol::lifecycle::{
    InitializeParams,
    InitializedParams,
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
    handler::MessageHandler,
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
    messages: MessageHandler,
    options: Arc<LSPServiceOptions>,
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
            messages: MessageHandler::new(),
            options,
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

        let params = InitializeParams::default();
        let Ok(payload) = self.messages.make_request("initialize", params, |messages, _| {
            let _ = messages.make_request_forget("initialized", InitializedParams);
        }) else {
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
                self.messages.handle_response(buffer);
            }
        }
    }
}
