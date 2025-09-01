use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use super::{
    popup::{
        KeaPopup,
        KeaPopupPosition,
        KeaPopupSize,
        PopupState,
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
                Query<(Entity, &mut KeaPopup)>,
                Query<(Entity, &mut Window)>,
                Query<(&mut Node, &mut Visibility)>,
                Commands,
            )> = SystemState::new(world);

            let (
                positions,
                mut popups,
                mut windows,
                mut nodes,
                mut commands,
            ) = system_state.get_mut(world);

            let Ok((popup_entity, mut popup)) = popups.single_mut() else {
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

                        if popup.window != Entity::PLACEHOLDER {
                            let window_position = positions
                                .map
                                .get(&window_entity)
                                .unwrap_or(&IVec2::ZERO);

                            // TODO: Get specific size of window title bar.
                            // May not be needed if a custom title bar is used.
                            result = window_position
                                + cursor_position.as_ivec2()
                                + IVec2::new(12, 36);
                        } else {
                            result = cursor_position.as_ivec2();
                        }

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

            if popup.window != Entity::PLACEHOLDER {
                let Ok((_, mut window)) = windows.get_mut(popup.window) else {
                    return;
                };

                window.position = WindowPosition::At(popup_position);
                window.resolution.set(window_size.x, window_size.y);
                window.visible = true;
            } else {
                let Ok((mut node, mut visibility)) = nodes.get_mut(popup_entity) else {
                    return;
                };

                node.left = Val::Px(popup_position.x as f32);
                node.top = Val::Px(popup_position.y as f32);
                node.width = Val::Px(window_size.x);
                node.height = Val::Px(window_size.y);
                *visibility = Visibility::Visible;
            }

            commands
                .entity(popup_entity)
                .despawn_related::<Children>()
                .add_child(entity);

            popup.state = PopupState::Opening;

            system_state.apply(world);
        });
        self.entity(entity)
    }

    fn kea_popup_close(&mut self) -> &mut Self {
        self.queue(|world: &mut World| {
            let mut system_state: SystemState<(
                Query<(Entity, &mut KeaPopup)>,
                Query<&mut Window>,
                Query<&mut Visibility>,
            )> = SystemState::new(world);

            let (
                mut popups,
                mut windows,
                mut visibilities,
            ) = system_state.get_mut(world);

            let Ok((popup_entity, mut popup)) = popups.single_mut() else {
                warn!("Failed to get single KeaPopup component.");
                return;
            };

            if popup.window != Entity::PLACEHOLDER {
                let Ok(mut window) = windows.get_mut(popup.window) else {
                    return;
                };

                window.visible = false;
            } else {
                let Ok(mut visibility) = visibilities.get_mut(popup_entity) else {
                    return;
                };

                *visibility = Visibility::Hidden;
            }

            popup.state = PopupState::Closing;

            system_state.apply(world);
        });
        self
    }
}
