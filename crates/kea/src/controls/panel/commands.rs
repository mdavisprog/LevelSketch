use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use crate::constants;
use super::component::KeaPanel;

pub trait KeaPanelCommands {
    fn kea_panel_focus(&mut self, panel: Entity) -> &mut Self;
}

impl<'w, 's> KeaPanelCommands for Commands<'w, 's> {
    fn kea_panel_focus(&mut self, panel: Entity) -> &mut Self {
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<Entity, With<KeaPanel>>,
                Query<&mut ZIndex>,
            )> = SystemState::new(world);

            let (
                panels,
                mut indices,
            ) = system_state.get_mut(world);

            for panel_entity in panels {
                let Ok(mut index) = indices.get_mut(panel_entity) else {
                    continue;
                };

                if panel == panel_entity {
                    index.0 = constants::PANEL_FOCUSED_Z_INDEX;
                } else {
                    index.0 = constants::BASE_Z_INDEX;
                }
            }

            system_state.apply(world);
        });
        self
    }
}
