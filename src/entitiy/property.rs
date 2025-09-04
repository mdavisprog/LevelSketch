use bevy::{
    platform::collections::HashMap,
    prelude::*,
};
use lstalk::prelude::*;
use super::data::EntityPropertyData;

pub type PropertyMap = HashMap<String, EntityProperty>;

pub struct EntityProperty {
    name: String,
    path: SymbolPath,
    data: EntityPropertyData,
    properties: PropertyMap,
}

impl EntityProperty {
    pub fn name(&self) -> &str {
        &self.name
    }

    pub fn path(&self) -> &SymbolPath {
        &self.path
    }

    pub fn data(&self) -> &EntityPropertyData {
        &self.data
    }

    pub fn data_mut(&mut self) -> &mut EntityPropertyData {
        &mut self.data
    }

    pub fn properties(&self) -> &PropertyMap {
        &self.properties
    }

    pub fn get_mut_property_from_parts(&mut self, parts: &mut Vec<&str>) -> Option<&mut EntityProperty> {
        if parts.is_empty() {
            Some(self)
        } else {
            let name = parts.remove(0);
            if let Some(property) = self.properties.get_mut(&name.to_string()) {
                property.get_mut_property_from_parts(parts)
            } else {
                None
            }
        }
    }
}

#[derive(Component, Default)]
pub struct EntityProperties {
    pub properties: PropertyMap,
}

impl EntityProperties {
    pub fn new() -> Self {
        Self {
            properties: PropertyMap::new(),
        }
    }

    pub fn contains(&self, name: &String) -> bool {
        self.properties.contains_key(name)
    }

    pub fn add(&mut self, symbol: &Symbol) -> &mut EntityProperty {
        let property = Self::create(symbol);
        self.properties.insert(symbol.name().to_string(), property);
        self.properties.get_mut(&symbol.name().to_string()).unwrap()
    }

    pub fn get_mut(&mut self, path: &SymbolPath) -> Option<&mut EntityProperty> {
        let mut parts = path.parts();
        if parts.is_empty() {
            None
        } else {
            let name = parts.remove(0);
            if let Some(property) = self.properties.get_mut(&name.to_string()) {
                property.get_mut_property_from_parts(&mut parts)
            } else {
                None
            }
        }
    }

    fn create(symbol: &Symbol) -> EntityProperty {
        let mut properties = PropertyMap::new();

        for (name, sub) in symbol.symbols() {
            let property = Self::create(sub);
            properties.insert(name.clone(), property);
        }

        let data = match symbol.data_type() {
            DataType::Decimal => EntityPropertyData::Decimal(0.0),
            _ => EntityPropertyData::None,
        };

        EntityProperty {
            name: symbol.name().to_string(),
            path: symbol.path().clone(),
            data,
            properties,
        }
    }
}
