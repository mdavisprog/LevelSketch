mod shapes;

use bevy::prelude::*;
pub use shapes::Shapes;

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        shapes::build(app);
    }
}
