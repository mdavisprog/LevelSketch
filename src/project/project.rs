use bevy::prelude::*;
use std::{
    io,
    path::Path,
};

pub struct Project {
    folder: String,
}

impl Project {
    pub fn open(&mut self, folder: &str) -> Result<(), io::Error> {
        let path = Path::new(folder);

        if !path.is_dir() {
            return Err(io::Error::new(io::ErrorKind::InvalidFilename,
                format!("Given path {folder} is not a directory!")));
        }

        info!("Successfully opened project at location: {folder}.");

        self.folder = folder.to_string();

        Ok(())
    }
}

impl Default for Project {
    fn default() -> Self {
        Self {
            folder: String::new(),
        }
    }
}

#[derive(Resource, Default)]
pub struct ProjectResource {
    pub project: Project,
}
