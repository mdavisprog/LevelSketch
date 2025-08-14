use std::{
    sync::{
        Arc,
        mpsc::{
            self,
            Sender,
        },
    },
    thread::{
        self,
        JoinHandle,
    },
};
use super::{
    errors::LSPServiceError,
    server::LanguageServer,
};

pub enum LSPServiceMessage {
    Shutdown,
    RequestTypes(Vec<String>),
}

#[derive(Default)]
pub struct LSPServiceOptions {
    pub print_stderr: bool,
    pub print_stdout: bool,
    pub print_send: bool,
}

pub struct LSPService {
    handle: Option<JoinHandle<()>>,
    sender: Option<Sender<LSPServiceMessage>>,
    options: Arc<LSPServiceOptions>,
}

impl LSPService {
    pub fn new() -> Self {
        Self::new_with_options(LSPServiceOptions::default())
    }

    pub fn new_with_options(options: LSPServiceOptions) -> Self {
        Self {
            handle: None,
            sender: None,
            options: Arc::new(options),
        }
    }

    pub fn start(&mut self, program: &str) -> Result<(), LSPServiceError> {
        let (sender, receiver) = mpsc::channel::<LSPServiceMessage>();

        let options = self.options.clone();
        let program_path = program.to_string();
        let Ok(result) = thread::Builder::new().name(format!("LSP Service")).spawn(move || {
            let mut server = match LanguageServer::spawn(&program_path, options) {
                Ok(server) => {
                    server
                },
                Err(error) => {
                    println!("Failed to spawn language server program '{program_path}': {error}.");
                    return;
                }
            };

            server.run(receiver);
        }) else {
            return Err(LSPServiceError::FailedToStart);
        };

        self.handle = Some(result);
        self.sender = Some(sender);

        Ok(())
    }

    pub fn stop(&mut self) -> Result<(), LSPServiceError> {
        println!("Shutting down LSP service.");

        let Some(handle) = self.handle.take() else {
            return Err(LSPServiceError::DidNotStart);
        };

        let Some(sender) = self.sender.take() else {
            return Err(LSPServiceError::DidNotStart);
        };

        if let Err(_) = sender.send(LSPServiceMessage::Shutdown) {
            return Err(LSPServiceError::DidNotStart);
        }

        let Ok(_) = handle.join() else {
            return Ok(())
        };

        Ok(())
    }

    pub fn request_types(&self, paths: Vec<String>) -> Result<(), LSPServiceError> {
        let Some(sender) = &self.sender else {
            return Err(LSPServiceError::DidNotStart);
        };

        match sender.send(LSPServiceMessage::RequestTypes(paths)) {
            Ok(_) => {},
            Err(_) => {
                return Err(LSPServiceError::FailedToSendMessage);
            }
        }

        Ok(())
    }
}
