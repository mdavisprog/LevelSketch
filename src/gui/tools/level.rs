use bevy::{
    ecs::{
        component::HookContext,
        world::DeferredWorld,
    },
    prelude::*,
};
use crate::level::{
    Level,
    LevelEventAddEntity,
};
use kea::prelude::*;
use super::tools::Tools;

#[derive(Component)]
#[require(
    Node = Self::node(),
    Tools,
)]
#[component(
    on_add = Self::on_add,
)]
pub struct LevelTools {
    _private: (),
}

impl LevelTools {
    pub fn bundle() -> impl Bundle {
        Self {
            _private: (),
        }
    }

    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(kea::style::properties::ROW_GAP),
            width: Val::Percent(100.0),
            ..default()
        }
    }

    fn on_add(
        mut world: DeferredWorld,
        HookContext {
            ..
        }: HookContext,
    ) {
        world
            .commands()
            .trigger(Refresh);
    }
}

#[derive(Component)]
struct LevelTreeRoot;

#[derive(Event)]
struct Refresh;

pub(super) fn build(app: &mut App) {
    app
        .add_observer(on_refresh)
        .add_systems(Update, on_entity_added)
        .add_systems(PostUpdate, on_level_added);
}

fn get_name(
    entity: Entity,
    names: &Query<&Name>,
) -> String {
    if let Ok(name) = names.get(entity) {
        name.as_str().to_string()
    } else {
        format!("Entity ({})", entity)
    }
}

fn spawn_hierarchy(
    parent_entity: Entity,
    parent_tree: Entity,
    parents: &Query<&Children>,
    names: &Query<&Name>,
    commands: &mut Commands,
) {
    let Ok(parent) = parents.get(parent_entity) else {
        return;
    };

    for child in parent {
        let name = get_name(*child, names);
        let id = commands.kea_tree_spawn_child(
            parent_tree,
            KeaTree::bundle_with_label(&name, ()),
        ).id();

        spawn_hierarchy(
            *child,
            id,
            parents,
            names,
            commands
        );
    }
}

fn on_refresh(
    _: Trigger<Refresh>,
    levels: Query<Entity, With<Level>>,
    parents: Query<&Children>,
    tools: Query<Entity, With<LevelTools>>,
    names: Query<&Name>,
    mut commands: Commands,
) {
    for tool in tools {
        commands
            .entity(tool)
            .despawn_related::<Children>();

        for level in levels {
            let name = get_name(level, &names);

            let id = commands
                .spawn((
                    KeaTree::bundle_with_label(&name, ()),
                    LevelTreeRoot,
                ))
                .id();

            commands
                .entity(tool)
                .add_child(id);

            spawn_hierarchy(
                level,
                id,
                &parents,
                &names,
                &mut commands
            );
        }
    }
}

fn on_level_added(
    levels: Query<Entity, Added<Level>>,
    mut commands: Commands,
) {
    if levels.is_empty() {
        return;
    }

    commands.trigger(Refresh);
}

fn on_entity_added(
    roots: Query<Entity, With<LevelTreeRoot>>,
    names: Query<&Name>,
    mut events: EventReader<LevelEventAddEntity>,
    mut commands: Commands,
) {
    for event in events.read() {
        for root in roots {
            let name = get_name(event.entity, &names);

            commands.kea_tree_spawn_child(
                root,
                KeaTree::bundle_with_label(&name, ()),
            );
        }
    }
}
