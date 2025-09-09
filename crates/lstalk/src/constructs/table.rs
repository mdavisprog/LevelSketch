use std::collections::HashMap;
use super::symbol::Symbol;

type Table = HashMap<String, Symbol>;

pub struct SymbolTable {
    table: Table,
}

impl SymbolTable {
    pub fn is_empty(&self) -> bool {
        self.table.is_empty()
    }

    pub fn sorted_names(&self) -> Vec<&String> {
        let mut result: Vec<&String> = self.table.keys().collect();
        result.sort_unstable();
        result
    }

    pub fn get(&self, name: &String) -> Option<&Symbol> {
        self.table.get(name)
    }

    pub(crate) fn new() -> Self {
        Self {
            table: Table::new(),
        }
    }

    pub(crate) fn insert(&mut self, name: String, symbol: Symbol) -> &mut Self {
        self.table.insert(name, symbol);
        self
    }

    pub(crate) fn get_mut(&mut self, name: &String) -> Option<&mut Symbol> {
        self.table.get_mut(name)
    }

    pub(crate) fn remove(&mut self, name: &String) -> Option<Symbol> {
        self.table.remove(name)
    }
}

impl<'a> IntoIterator for &'a SymbolTable {
    type Item = <&'a Table as IntoIterator>::Item;
    type IntoIter = <&'a Table as IntoIterator>::IntoIter;

    fn into_iter(self) -> Self::IntoIter {
        (&self.table).into_iter()
    }
}
