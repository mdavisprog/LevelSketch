/// Represents a full path to a symbol. This is represented by {parent}::{child}.
#[derive(PartialEq, Eq, Clone)]
pub struct SymbolPath {
    path: String,
}

impl SymbolPath {
    const PATTERN: &str = "::";

    pub fn new(path: String) -> Self {
        Self {
            path,
        }
    }

    pub fn new_with_slice(paths: &[&str]) -> Self {
        Self {
            path: paths.join(Self::PATTERN),
        }
    }

    pub fn new_with_prepend(path: String, other: &SymbolPath) -> Self {
        let mut result = Self::new(path);
        result.prepend(other);
        result
    }

    pub fn full_path(&self) -> &str {
        &self.path
    }

    pub fn name(&self) -> &str {
        let splits: Vec<&str> = self.path.rsplit(Self::PATTERN).collect();

        if splits.is_empty() {
            ""
        } else {
            splits.first().unwrap()
        }
    }

    pub fn parts(&self) -> Vec<&str> {
        self.path.split(Self::PATTERN).collect()
    }

    pub fn prepend(&mut self, other: &SymbolPath) -> &mut Self {
        self.path.insert_str(0, Self::PATTERN);
        self.path.insert_str(0, &other.path);
        self
    }
}

impl std::fmt::Display for SymbolPath {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.path)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn new() {
        let path = SymbolPath::new(format!("root"));
        assert!(path.path == "root");
    }

    #[test]
    fn new_with_slice() {
        let path = SymbolPath::new_with_slice(&[
            "hello",
            "foo",
            "bar",
        ]);
        assert_eq!(path.path, "hello::foo::bar");
    }

    #[test]
    fn new_with_prepend() {
        let foo = SymbolPath::new(format!("foo"));
        let bar = SymbolPath::new_with_prepend(format!("bar"), &foo);

        assert_eq!(foo.path, "foo");
        assert_eq!(bar.path, "foo::bar");
    }

    #[test]
    fn prepend() {
        let foo = SymbolPath::new(format!("foo"));
        let mut bar = SymbolPath::new(format!("bar"));
        bar.prepend(&foo);
        assert_eq!(bar.path, "foo::bar");
    }

    #[test]
    fn name() {
        let foo = SymbolPath::new(format!("foo"));
        let mut bar = SymbolPath::new(format!("bar"));

        assert_eq!(foo.name(), "foo");
        assert_eq!(bar.name(), "bar");

        bar.prepend(&foo);
        assert_eq!(bar.path, "foo::bar");
        assert_eq!(bar.name(), "bar");

        let empty = SymbolPath::new(format!(""));
        assert_eq!(empty.path, "");
    }
}
