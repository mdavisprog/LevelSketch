pub enum LSPServiceError {
    FailedToStart,
    DidNotStart,
    FailedToSendMessage,
}

impl std::fmt::Display for LSPServiceError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Self::FailedToStart => write!(f, "The LSP service failed to start"),
            Self::DidNotStart => write!(f, "The LSP service did not start"),
            Self::FailedToSendMessage =>
                write!(f, "The LSP service failed to send a message to the server"),
        }
    }
}

pub enum LanguageServerError {
    FailedToSpawn,
}

impl std::fmt::Display for LanguageServerError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Self::FailedToSpawn => write!(f, "The language server failed to spawn"),
        }
    }
}
