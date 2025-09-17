pub type DocumentUri = String;
pub type URI = String;

const PREFIX: &str = "file:///";

pub fn make_file_uri(uri: &str) -> DocumentUri {
    if uri.starts_with(PREFIX) {
        uri.to_string()
    } else {
        format!("{PREFIX}{uri}")
    }
}
