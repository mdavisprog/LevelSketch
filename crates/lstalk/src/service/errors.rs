#[derive(Debug)]
pub enum LSPServiceError {
    FailedToStart,
    DidNotStart,
    FailedToSendMessage,
}

#[derive(Debug)]
pub enum LanguageServerError {
    FailedToSpawn,
    FailedToSendMessage,
    DidNotState,
    FailedToStop,
}
