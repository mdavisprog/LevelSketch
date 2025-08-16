use crate::protocol::document::SemanticToken;
use std::{
    fs,
    io::{
        BufRead,
        BufReader,
        self,
    },
};

/// The URI and the contents of a document in memory. Useful for querying information such
/// as symbol names through line numbers and indices.
pub struct Document {
    lines: Vec<String>,
}

impl Document {
    pub fn open(path: &str) -> Result<Self, io::Error> {
        let file = fs::File::open(path)?;
        let mut reader = BufReader::new(file);

        let mut lines = Vec::<String>::new();
        loop {
            let mut line = String::new();

            let Ok(size) = reader.read_line(&mut line) else {
                break;
            };

            if size == 0 {
                break;
            }

            lines.push(line);
        }

        Ok(Self {
            lines,
        })
    }

    pub fn resolve(&self, tokens: &Vec<SemanticToken>) -> Vec<ResolvedToken> {
        let mut result = Vec::<ResolvedToken>::new();

        let mut line = 0;
        let mut start = 0;
        for token in tokens {
            if token.line == 0 {
                start += token.start;
            } else {
                start = token.start;
            }

            line += token.line;

            let end = start + token.length;
            if let Some(name) = self.get_name(line, start, end) {
                result.push(ResolvedToken {
                    name,
                    token_type: token.token_type,
                    token_modifiers: token.modifiers,
                });
            }
        }

        result
    }

    fn get_name(
        &self,
        line_number: usize,
        start: usize,
        end: usize,
    ) -> Option<String> {
        let line = &self.lines[line_number];

        let Some(token) = line.get(start..end) else {
            return None;
        };

        Some(token.to_string())
    }
}

pub struct ResolvedToken {
    pub name: String,
    pub token_type: usize,
    pub token_modifiers: usize,
}
