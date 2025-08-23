use bevy::prelude::*;
use crate::project::{
    LSPEvent,
    LSPServiceResource,
};
use kea::prelude::*;
use lstalk::prelude::*;
use super::tools::Tools;

#[derive(Component)]
#[require(
    Node = Self::node(),
    Tools,
)]
pub struct TypesTool {
    _private: (),
}

impl TypesTool {
    pub fn bundle() -> impl Bundle {
        Self {
            _private: (),
        }
    }

    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Column,
            row_gap: Val::Px(kea::style::properties::ROW_GAP),
            ..default()
        }
    }
}

#[derive(Component)]
pub struct TypesPanel {
    _private: (),
}

impl TypesPanel {
    #[expect(unused)]
    pub fn bundle(position: Vec2, size: Vec2) -> impl Bundle {(
        Self {
            _private: (),
        },
        KeaPanel::bundle(KeaPanelOptions {
            title: format!("Types"),
            position,
            size,
        },
        (
            Node {
                flex_direction: FlexDirection::Column,
                row_gap: Val::Px(kea::style::properties::ROW_GAP),
                width: Val::Percent(100.0),
                height: Val::Percent(100.0),
                ..default()
            },
            KeaScrollable,
            children![
                TypesTool::bundle(),
            ],
        )),
    )}
}

pub(super) fn build(app: &mut App) {
    app.add_observer(on_symbols);
}

fn on_symbols(
    trigger: Trigger<LSPEvent>,
    lsp_resource: Res<LSPServiceResource>,
    types_tool: Query<Entity, With<TypesTool>>,
    mut commands: Commands,
) {
    let event = trigger.event();

    match event {
        LSPEvent::Symbols => {
            for tool in types_tool {
                commands
                    .entity(tool)
                    .despawn_related::<Children>();
            }

            let Some(symbols) = lsp_resource.symbols() else {
                return;
            };

            for (name, symbol) in symbols {
                if symbol.kind() == SymbolKind::Class {
                    for tool in types_tool {
                        let parent = commands.spawn(KeaTree::bundle_with_label(&name, ())).id();
                        commands
                            .entity(tool)
                            .add_child(parent);

                        spawn_symbols(symbol, parent, &mut commands);
                    }
                }
            }
        },
    }
}

fn spawn_symbols(
    root: &Symbol,
    parent: Entity,
    commands: &mut Commands,
) {
    for (name, symbol) in root.symbols() {
        let entity = commands
            .kea_tree_spawn_child(parent, KeaTree::bundle_with_label(&name, ()))
            .id();

        spawn_symbols(symbol, entity, commands);
    }
}
