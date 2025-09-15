use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use super::expander::{
    Contents,
    Header,
    KeaExpander,
};

pub trait KeaExpanderCommands {
    fn kea_expander_set_label(&mut self, expander: Entity, label: String) -> &mut Self;
    fn kea_expander_spawn_contents(&mut self, expander: Entity, contents: impl Bundle) -> &mut Self;
    fn kea_expander_add_contents(&mut self, expander: Entity, contents: Vec<Entity>) -> &mut Self;
}

impl<'w, 's> KeaExpanderCommands for Commands<'w, 's> {
    fn kea_expander_set_label(&mut self, expander: Entity, label: String) -> &mut Self {
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&KeaExpander>,
                Query<&Header>,
                Query<&Children>,
                Query<&mut Text>,
            )> = SystemState::new(world);

            let (
                expanders,
                headers,
                parents,
                mut texts,
            ) = system_state.get_mut(world);

            if !expanders.contains(expander) {
                warn!("Given entity {expander} is not a KeaExpander.");
                return;
            }

            let Ok(expander_parent) = parents.get(expander) else {
                warn!("Given KeaExpander {expander} has no children.");
                return;
            };

            let mut text_entity = Entity::PLACEHOLDER;
            for child in expander_parent {
                if !headers.contains(*child) {
                    continue;
                }

                let Ok(header) = parents.get(*child) else {
                    continue;
                };

                for header_child in header {
                    if texts.contains(*header_child) {
                        text_entity = *header_child;
                        break;
                    }
                }
                break;
            }

            let Ok(mut text) = texts.get_mut(text_entity) else {
                warn!("Failed to find header label for KeaExpander {expander}.");
                return;
            };

            text.0 = label;

            system_state.apply(world);
        });
        self
    }

    fn kea_expander_spawn_contents(&mut self, expander: Entity, contents: impl Bundle) -> &mut Self {
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&KeaExpander>,
                Query<&Contents>,
                Query<&Children>,
                Commands,
            )> = SystemState::new(world);

            let (
                expanders,
                contents_components,
                parents,
                mut commands,
            ) = system_state.get_mut(world);

            if !expanders.contains(expander) {
                warn!("Given entity {expander} is not a KeaExpander.");
                return;
            }

            for child in parents.iter_descendants(expander) {
                if !contents_components.contains(child) {
                    continue;
                }

                commands
                    .entity(child)
                    .despawn_related::<Children>()
                    .with_child(contents);

                break;
            }

            system_state.apply(world);
        });
        self
    }

    fn kea_expander_add_contents(&mut self, expander: Entity, contents: Vec<Entity>) -> &mut Self {
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&KeaExpander>,
                Query<&Contents>,
                Query<&Children>,
                Commands,
            )> = SystemState::new(world);

            let (
                expanders,
                contents_components,
                parents,
                mut commands,
            ) = system_state.get_mut(world);

            if !expanders.contains(expander) {
                warn!("Given entity {expander} is not a KeaExpander.");
                return;
            }

            for child in parents.iter_descendants(expander) {
                if !contents_components.contains(child) {
                    continue;
                }

                commands
                    .entity(child)
                    .despawn_related::<Children>()
                    .add_children(&contents);

                break;
            }

            system_state.apply(world);
        });
        self
    }
}
