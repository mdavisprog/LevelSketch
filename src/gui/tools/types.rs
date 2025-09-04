use bevy::prelude::*;
use crate::project::LSPServiceResource;
use kea::prelude::*;
use super::tools::Tools;

#[derive(Component)]
#[require(Tools)]
pub struct TypesTool {
    _private: (),
}

impl TypesTool {
    pub fn bundle() -> impl Bundle {
        Self {
            _private: (),
        }
    }
}

#[derive(Event)]
pub struct TypeSelected {
    pub name: String,
}

#[derive(Event)]
struct Refresh;

pub(super) fn build(app: &mut App) {
    app
        .add_observer(on_add)
        .add_observer(refresh);
}

fn on_add(
    _: Trigger<OnAdd, TypesTool>,
    mut commands: Commands,
) {
    commands.trigger(Refresh);
}

fn refresh(
    _: Trigger<Refresh>,
    types_tools: Query<Entity, With<TypesTool>>,
    lsp_resource: Res<LSPServiceResource>,
    mut commands: Commands,
) {
    for tool in types_tools {
        commands
            .entity(tool)
            .despawn_related::<Children>();

        let Some(symbols) = lsp_resource.symbols() else {
            commands
                .entity(tool)
                .with_child(warning_message(
                    "Language server has not been started. Please load a project to retrieve valid types."
                ));
            println!("Not initialized");
            return;
        };

        if symbols.is_empty() {
            commands
                .entity(tool)
                .with_child(warning_message(
                    "Language server did not find any types from the project."
                ));
            println!("No symbols");
            return;
        }

        let list = {
            let mut list = commands.spawn(KeaList::new(KeaListBehavior::Select));

            for name in symbols.sorted_names() {
                list.with_child(KeaLabel::bundle(&name));
            }

            list
                .observe(on_select)
                .id()
        };

        commands
            .entity(tool)
            .add_child(list);
    }
}

fn warning_message(message: &str) -> impl Bundle {(
    KeaLabel::bundle(message),
    TextLayout::new_with_justify(JustifyText::Center),
)}

fn on_select(
    trigger: Trigger<KeaListSelect>,
    children: Query<&ChildOf>,
    texts: Query<&Text>,
    tools: Query<&TypesTool>,
    mut commands: Commands,
) {
    let event = trigger.event();

    let mut tool = Entity::PLACEHOLDER;
    for parent in children.iter_ancestors(trigger.target()) {
        if tools.contains(parent) {
            tool = parent;
            break;
        }
    }

    assert!(tool != Entity::PLACEHOLDER);

    let Ok(text) = texts.get(event.entity) else {
        panic!("Entity {} is not a KeaLabel.", event.entity);
    };

    commands
        .kea_popup_close()
        .trigger_targets(TypeSelected {
            name: text.0.clone(),
        }, tool);
}
