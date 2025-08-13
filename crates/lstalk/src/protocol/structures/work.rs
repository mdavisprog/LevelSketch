use crate::protocol::base::ProgressToken;
use serde::{
    Deserialize,
    Serialize,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workDoneProgressParams
#[derive(Serialize, Deserialize, Default)]
pub struct WorkDoneProgressParams {
    // An optional token that a server can use to report work done progress.
    #[serde(rename = "workDoneToken")]
    work_done_token: Option<ProgressToken>,
}
