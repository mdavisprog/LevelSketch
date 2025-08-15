use bevy::prelude::*;
use lstalk::prelude::*;

#[derive(Resource)]
pub struct LSPServiceResource {
    pub service: LSPService,
}

impl Default for LSPServiceResource {
    fn default() -> Self {
        Self {
            service: LSPService::new_with_options(Self::options_from_command_line()),
        }
    }
}

impl Drop for LSPServiceResource {
    fn drop(&mut self) {
        self.service.stop();
    }
}

impl LSPServiceResource {
    pub fn start(&mut self) {
        let program = {
            let mut result = String::new();

            for arg in std::env::args() {
                if arg.starts_with("--lsp=") {
                    let tokens: Vec<&str> = arg.split("=").collect();
                    if tokens.len() > 1 {
                        result = tokens[1].into();
                        break;
                    }
                }
            }

            result
        };

        match self.service.start(&program) {
            Ok(_) => {},
            Err(error) => {
                warn!("Failed to start LSP service: {error:?}");
            }
        }
    }

    pub fn poll(&self) {
        self.service.poll();
    }

    fn options_from_command_line() -> LSPServiceOptions {
        let mut options = LSPServiceOptions::default();
        for arg in std::env::args() {
            match arg.as_str() {
                "--lsp-stderr" => options.print_stderr = true,
                "--lsp-stdout" => options.print_stdout = true,
                "--lsp-send" => options.print_send = true,
                _ => {},
            }
        }

        options
    }
}
