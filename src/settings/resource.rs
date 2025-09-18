use bevy::prelude::*;
use std::{
    io,
    path::PathBuf,
};
use super::app::AppSettings;

#[derive(Resource, Default)]
pub struct SettingsResource {
    pub app: AppSettings,
}

impl SettingsResource {
    pub(super) fn save(&self) -> Result<(), io::Error> {
        let contents = serde_json::to_string_pretty(&self.app)?;
        let path = self.get_path()?;
        std::fs::write(path, contents)
    }

    pub(super) fn load(&mut self) -> Result<(), io::Error> {
        let path = self.get_path()?;
        let contents = std::fs::read_to_string(path)?;
        self.app = serde_json::from_str(&contents)?;
        Ok(())
    }

    fn get_path(&self) -> Result<PathBuf, io::Error> {
        let exe_path = std::env::current_exe()?;
        let mut path = match exe_path.parent() {
            Some(parent) => {
                parent.to_path_buf()
            },
            None => {
                return Err(io::Error::new(
                    io::ErrorKind::InvalidFilename,
                    format!("Failed to retrieve directory for exe path {exe_path:?}"),
                ));
            }
        };

        path.push("settings.json");
        Ok(path)
    }
}
