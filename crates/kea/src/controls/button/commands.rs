use bevy::prelude::*;

pub(super) struct ChangeLabel {
    pub entity: Entity,
    pub label: String,
}

impl Command for ChangeLabel {
    fn apply(self, world: &mut World) {
        let children: Vec<Entity> = if let Ok(button) = world.get_entity(self.entity) {
            if let Some(children) = button.get::<Children>() {
                children.iter().collect()
            } else {
                Vec::new()
            }
        } else {
            Vec::new()
        };

        for child in children {
            let Ok(mut text_entity) = world.get_entity_mut(child) else {
                continue;
            };

            let Some(mut text) = text_entity.get_mut::<Text>() else {
                continue;
            };

            *text = self.label.clone().into();
        }
    }
}
