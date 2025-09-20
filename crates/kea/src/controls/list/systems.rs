use bevy::prelude::*;
use crate::{
    observers::KeaObservers,
    style,
};
use super::{
    commands::KeaListCommandsExt,
    component::{
        KeaList,
        KeaListItem,
        KeaListLabelItems,
    },
    events::{
        KeaListHover,
        KeaListSelect,
    },
};

pub(super) fn build(app: &mut App) {
    app
        .add_systems(Update, on_add_list_item)
        .add_observer(on_add_list_items);
}

fn on_add_list_items(
    trigger: Trigger<OnAdd, KeaListLabelItems>,
    list_label_items: Query<&KeaListLabelItems>,
    lists: Query<Entity, With<KeaList>>,
    mut commands: Commands,
) {
    let Ok(list_label_items) = list_label_items.get(trigger.target()) else {
        return;
    };

    let Ok(list) = lists.get(trigger.target()) else {
        return;
    };

    for item in &list_label_items.0 {
        commands
            .entity(list)
            .with_child((
                Text::new(item),
                TextFont::from_font_size(12.0),
            ));
    }

    commands
        .entity(list)
        .remove::<KeaListLabelItems>();
}

fn on_list_item_over(
    trigger: Trigger<Pointer<Over>>,
    children: Query<&Children>,
    parents: Query<&ChildOf>,
    mut background_colors: Query<&mut BackgroundColor>,
    mut commands: Commands,
) {
    let Ok(mut background_color) = background_colors.get_mut(trigger.target()) else {
        return;
    };

    *background_color = style::colors::HIGHLIGHT.into();

    let Ok(parent) = parents.get(trigger.target()) else {
        return;
    };

    let Ok(children) = children.get(parent.parent()) else {
        return;
    };

    let index = children
        .iter()
        .position(|element| element == trigger.target())
        .unwrap_or(0);

    commands.trigger_targets(KeaListHover {
        entity: trigger.target(),
        index,
    }, parent.parent());
}

fn on_list_item_out(
    trigger: Trigger<Pointer<Out>>,
    children: Query<&ChildOf>,
    lists: Query<(&KeaList, &Children)>,
    mut background_colors: Query<&mut BackgroundColor>,
) {
    let Ok(child) = children.get(trigger.target()) else {
        return;
    };

    let Ok((list, parent)) = lists.get(child.parent()) else {
        return;
    };

    for item in &list.selected {
        let Some(entity) = parent.get(*item) else {
            continue;
        };

        if *entity == trigger.target() {
            return;
        }
    }

    let Ok(mut background_color) = background_colors.get_mut(trigger.target()) else {
        return;
    };

    *background_color = Color::NONE.into();
}

fn on_list_item_pressed(
    trigger: Trigger<Pointer<Pressed>>,
    parents: Query<&Children>,
    children: Query<&ChildOf>,
    mut commands: Commands,
) {
    let Ok(child) = children.get(trigger.target()) else {
        return;
    };

    let Ok(parent) = parents.get(child.parent()) else {
        return;
    };

    let index = parent
        .iter()
        .position(|element| element == trigger.target())
        .unwrap_or(usize::MAX);

    if index >= parent.len() {
        return;
    }

    commands
        .kea_list_select(child.parent(), index)
        .trigger_targets(KeaListSelect {
            entity: trigger.target(),
            index,
        }, child.parent());
}

fn on_add_list_item(
    lists: Query<&Children, (Changed<Children>, With<KeaList>)>,
    items: Query<&KeaListItem>,
    mut commands: Commands,
) {
    for list in &lists {
        for child in list {
            if items.contains(*child) {
                continue;
            }

            commands
                .entity(*child)
                .insert((
                    KeaObservers::<KeaListItem>::new(vec![
                        Observer::new(on_list_item_over),
                        Observer::new(on_list_item_out),
                        Observer::new(on_list_item_pressed),
                    ]),
                    KeaListItem,
                ));
        }
    }
}
