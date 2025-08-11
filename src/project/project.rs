use bevy::prelude::*;
use std::{
    io,
    path::{
        Path,
        PathBuf,
    },
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

    pub fn gather_source_files(&self) -> Vec<String> {
        self.retrieve_files(&self.folder)
    }

    fn retrieve_files(&self, path: &str) -> Vec<String> {
        let mut result = Vec::<String>::new();

        let dirs = match std::fs::read_dir(path) {
            Ok(result) => result,
            Err(error) => {
                println!("Failed to read directory '{path}'. Error: {error:?}");
                return result;
            }
        };

        for dir in dirs {
            let entry = match dir {
                Ok(result) => result,
                Err(error) => {
                    println!("Failed to read directory entry: {error:?}");
                    continue;
                }
            };

            let file_type = match entry.file_type() {
                Ok(result) => result,
                Err(error) => {
                    println!("Failed to get file type for '{:?}'. Error: {error:?}", entry.file_name());
                    continue;
                }
            };

            let Ok(file_name) = entry.file_name().into_string() else {
                println!("Failed to convert OsStr into string: {:?}!", entry.file_name());
                continue;
            };

            let mut file_path = PathBuf::new();
            file_path.set_file_name(path);
            file_path.push(file_name);

            let file_path_str = file_path
                .as_os_str()
                .to_str()
                .unwrap_or_default();

            if file_type.is_dir() {
                let mut directory_files = self.retrieve_files(file_path_str);
                result.append(&mut directory_files);
            } else if file_type.is_file() {
                result.push(file_path_str.to_string());
            } else {
                panic!("Symlink directories are not handled!");
            }
        }

        result
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
