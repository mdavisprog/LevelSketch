use crate::protocol::{
    document::SemanticToken,
    structures::Range,
};
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

    pub fn get_contents_from_range(&self, range: Range) -> Option<String> {
        if range.start.line == range.end.line {
            let Some(line) = self.lines.get(range.start.line as usize) else {
                return None;
            };

            let start = range.start.character as usize;
            let end= range.end.character as usize;

            Some(line[start..end].to_string())
        } else {
            let min_line = range.start.line.min(range.end.line) as usize;
            let max_line = range.start.line.max(range.end.line) as usize;

            let (min_char, max_char) = if range.start.line < range.end.line {
                (range.start.character as usize, range.end.character as usize)
            } else {
                (range.end.character as usize, range.start.character as usize)
            };

            let mut result = String::new();

            for index in min_line..=max_line {
                let line = &self.lines[index];

                if index == min_line {
                    result += &line[min_char..];
                } else if index == max_line {
                    result += &line[..max_char];
                } else {
                    result += line;
                }
            }

            Some(result)
        }
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
