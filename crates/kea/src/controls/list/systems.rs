use bevy::prelude::*;
use crate::{
    observers::KeaObservers,
    style,
};
use super::{
    component::{
        KeaList,
        KeaListItem,
        KeaListLabelItems,
    },
    events::{
        KeaListHover,
        KeaListSelect,
    },
    extensions::KeaListCommandsExt,
};

pub(super) fn build(app: &mut App) {
    app.add_observer(on_add_list_items);
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
                KeaObservers::<KeaListItem>::new(vec![
                    Observer::new(on_list_item_over),
                    Observer::new(on_list_item_out),
                    Observer::new(on_list_item_pressed),
                ]),
                KeaListItem,
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
    lists: Query<&KeaList>,
    mut background_colors: Query<&mut BackgroundColor>,
) {
    let Ok(child) = children.get(trigger.target()) else {
        return;
    };

    let Ok(list) = lists.get(child.parent()) else {
        return;
    };

    if list.selected.contains(&trigger.target()) {
        return;
    }

    let Ok(mut background_color) = background_colors.get_mut(trigger.target()) else {
        return;
    };

    *background_color = Color::NONE.into();
}

fn on_list_item_pressed(
    trigger: Trigger<Pointer<Pressed>>,
    children: Query<&Children>,
    parents: Query<&ChildOf>,
    mut commands: Commands,
) {
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

    commands
        .kea_list_select(parent.parent(), &[trigger.target()])
        .trigger_targets(KeaListSelect {
            entity: trigger.target(),
            index,
        }, parent.parent());
}
