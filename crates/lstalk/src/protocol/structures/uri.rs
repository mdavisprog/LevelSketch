pub type DocumentUri = String;
pub type URI = String;

pub fn make_file_uri(uri: &str) -> DocumentUri {
    format!("file:///{uri}")
}
