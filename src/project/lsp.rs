use bevy::prelude::*;
use lstalk::prelude::*;

#[derive(Resource)]
pub(super) struct LSPServiceResource {
    service: LSPService,
}

impl Default for LSPServiceResource {
    fn default() -> Self {
        Self {
            service: LSPService::new_with_options(LSPServiceOptions {
                print_stderr: false,
                ..default()
            }),
        }
    }
}

impl Drop for LSPServiceResource {
    fn drop(&mut self) {
        match self.service.stop() {
            Ok(_) => {},
            Err(error) => {
                error!("Failed to join LSP service: {error}.");
            },
        }
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
                warn!("Failed to start LSP service: {error}");
            }
        }
    }
}
