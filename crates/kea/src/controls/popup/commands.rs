use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use super::{
    popup::{
        KeaPopup,
        KeaPopupPosition,
        KeaPopupSize,
    },
    systems::WindowPositions,
};

pub trait KeaPopupCommands {
    fn kea_popup_open(
        &mut self,
        bundle: impl Bundle,
        position: KeaPopupPosition,
        size: KeaPopupSize,
    ) -> EntityCommands<'_>;

    fn kea_popup_close(&mut self) -> &mut Self;
}

impl<'w, 's> KeaPopupCommands for Commands<'w, 's> {
    fn kea_popup_open(
        &mut self,
        bundle: impl Bundle,
        position: KeaPopupPosition,
        size: KeaPopupSize,
    ) -> EntityCommands<'_> {
        let entity = self.spawn(bundle).id();
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Res<WindowPositions>,
                Query<(Entity, &KeaPopup)>,
                Query<(Entity, &mut Window)>,
                Commands,
            )> = SystemState::new(world);

            let (
                positions,
                popups,
                mut windows,
                mut commands,
            ) = system_state.get_mut(world);

            let Ok((popup_entity, popup)) = popups.single() else {
                warn!("Failed to get single KeaPopup component.");
                return;
            };

            let popup_position = match position {
                KeaPopupPosition::At(position) => {
                    position
                },
                KeaPopupPosition::AtMouse => {
                    let mut result = IVec2::ZERO;

                    for (window_entity, window) in &windows {
                        let Some(cursor_position) = window.cursor_position() else {
                            continue;
                        };

                        let window_position = positions
                            .map
                            .get(&window_entity)
                            .unwrap_or(&IVec2::ZERO);

                        // TODO: Get specific size of window title bar.
                        // May not be needed if a custom title bar is used.
                        result = window_position
                            + cursor_position.as_ivec2()
                            + IVec2::new(12, 36);
                        break;
                    }

                    result
                },
            };

            let window_size = match size {
                KeaPopupSize::Fixed(size) => {
                    size
                },
            };

            let Ok((_, mut window)) = windows.get_mut(popup.window) else {
                return;
            };

            commands
                .entity(popup_entity)
                .despawn_related::<Children>();

            window.position = WindowPosition::At(popup_position);
            window.resolution.set(window_size.x, window_size.y);
            window.visible = true;

            commands
                .entity(popup_entity)
                .add_child(entity);

            system_state.apply(world);
        });
        self.entity(entity)
    }

    fn kea_popup_close(&mut self) -> &mut Self {
        self.queue(|world: &mut World| {
            let mut system_state: SystemState<(
                Query<&KeaPopup>,
                Query<&mut Window>,
            )> = SystemState::new(world);

            let (
                popups,
                mut windows,
            ) = system_state.get_mut(world);

            let Ok(popup) = popups.single() else {
                warn!("Failed to get single KeaPopup component.");
                return;
            };

            let Ok(mut window) = windows.get_mut(popup.window) else {
                return;
            };

            window.visible = false;

            system_state.apply(world);
        });
        self
    }
}
