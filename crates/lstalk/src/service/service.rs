use std::path::Path;
use super::{
    errors::LSPServiceError,
    server::{
        LanguageServer,
        LanguageServerEvent,
        LanguageServerMessage,
    },
};

pub enum LSPServiceMessage {
    Shutdown,
    RequestTypes(Vec<String>),
}

#[derive(Default, Clone, Copy)]
pub struct LSPServiceOptions {
    pub print_stderr: bool,
    pub print_stdout: bool,
    pub print_send: bool,
}

pub struct LSPService {
    options: LSPServiceOptions,
    servers: Vec<LanguageServer>,
}

impl LSPService {
    pub fn new() -> Self {
        Self::new_with_options(LSPServiceOptions::default())
    }

    pub fn new_with_options(options: LSPServiceOptions) -> Self {
        Self {
            options: options,
            servers: Vec::new(),
        }
    }

    pub fn start(
        &mut self,
        program: &str,
        workspace_folder: &str,
    ) -> Result<(), LSPServiceError> {
        let name = if let Some(stem) = Path::new(program).file_stem() {
            stem
                .to_str()
                .unwrap_or_else(|| program)
                .to_string()
        } else {
            program.to_string()
        };

        let mut server = LanguageServer::new()
            .set_program(program.to_string())
            .set_name(name)
            .set_workspace_folder(workspace_folder.to_string());

        match server.run(self.options.clone()) {
            Ok(_) => {
                self.servers.push(server);
            },
            Err(error) => {
                println!("Failed to launch and run language server: {error:?}.");
                return Err(LSPServiceError::FailedToStart);
            },
        }

        Ok(())
    }

    pub fn stop(&mut self) {
        println!("Shutting down LSP service.");
        self.broadcast_message(LanguageServerMessage::Shutdown);

        for mut server in self.servers.drain(..) {
            if let Err(error) = server.stop() {
                println!("{error:?}.");
            }
        }
    }

    pub fn poll(&self) -> LSPServicePollResult {
        let mut result = LSPServicePollResult::new();

        for server in &self.servers {
            if let Some(event) = server.poll() {
                result.items.push(LSPServicePollResultItem {
                    server: server,
                    event,
                });
            }
        }

        result
    }

    pub fn request_types(&mut self, paths: Vec<String>) {
        self.broadcast_message(LanguageServerMessage::RequestTypes(paths));
    }

    fn broadcast_message(&mut self, message: LanguageServerMessage) {
        for server in &mut self.servers {
            if let Err(error) = server.send_message(message.clone()) {
                println!("{error:?}.");
            }
        }
    }
}

pub struct LSPServicePollResult<'a> {
    pub items: Vec<LSPServicePollResultItem<'a>>,
}

impl<'a> LSPServicePollResult<'a> {
    fn new() -> Self {
        Self {
            items: Vec::new(),
        }
    }
}

pub struct LSPServicePollResultItem<'a> {
    pub server: &'a LanguageServer,
    pub event: LanguageServerEvent,
}
