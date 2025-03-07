use bevy::prelude::*;
use super::style;

//
// Public API
//

#[derive(Event, PartialEq, Eq)]
pub enum MenuItemEvent {
    Enter,
    Leave,
    Pressed,
}

pub fn create_menu_bar(
    commands: &mut Commands,
    items: Vec<(&str, Observer)>,
) {
    commands.spawn(
        MenuBar
    )
    .with_children(|parent| {
        for (text, mut observer) in items {
            let mut entity = parent.spawn(MenuBarItem);

            entity.with_children(|parent| {
                parent.spawn((
                    MenuItemLabel,
                    Text::new(text),
                    TextLayout::new_with_justify(JustifyText::Center),
                ));
            });

            MenuItem::add_observers(&mut entity);

            observer.watch_entity(entity.id());
            entity.with_child(observer);
        }
    });
}

pub fn create_menu(
    commands: &mut Commands,
    items: Vec<(&str, Observer)>,
    menu_item: Entity,
) {
    commands.spawn(
        Menu {
            menu_item,
        }
    )
    .with_children(|parent| {
        for (text, mut observer) in items {
            let mut entity = parent.spawn((
                MenuItem,
                Node {
                    padding: UiRect::axes(Val::Px(20.0), Val::Px(4.0)),
                    ..default()
                }
            ));
    
            entity.with_children(|parent| {
                parent.spawn((
                    MenuItemLabel,
                    Text::new(text),
                ));
            });

            MenuItem::add_observers(&mut entity);

            observer.watch_entity(entity.id());
            entity.with_child(observer);
        }
    });
}

pub fn close_menus(commands: &mut Commands) {
    commands.trigger(MenuEvent::CloseAll);
}

//
// Types
//

pub struct Plugin;

impl bevy::prelude::Plugin for Plugin {
    fn build(&self, app: &mut App) {
        app
            .insert_resource(State)
            .add_observer(Menu::on_added)
            .add_observer(Menu::observe_close_all);
    }
}

#[derive(Resource)]
struct State;

#[derive(Component)]
#[require(
    Node(MenuBar::node),
    BackgroundColor(|| style::colors::BACKGROUND)
)]
struct MenuBar;

impl MenuBar {
    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(3.0),
            justify_content: JustifyContent::Start,
            flex_direction: FlexDirection::Row,
            column_gap: Val::Px(0.0),
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Text,
    TextFont(MenuItemLabel::text_font),
    PickingBehavior(|| PickingBehavior::IGNORE),
)]
struct MenuItemLabel;

impl MenuItemLabel {
    fn text_font() -> TextFont {
        TextFont {
            font_size: 12.0,
            ..default()
        }
    }
}

#[derive(Component)]
#[require(
    Node(MenuItem::node),
    BackgroundColor(|| style::colors::BACKGROUND),
    BorderColor(|| Color::srgb(1.0, 0.0, 0.0)),
)]
struct MenuItem;

#[derive(Component)]
#[require(MenuItem)]
struct MenuBarItem;

impl MenuItem {
    fn node() -> Node {
        Node {
            justify_content: JustifyContent::Center,
            align_content: AlignContent::Center,
            justify_self: JustifySelf::Stretch,
            align_self: AlignSelf::Stretch,
            padding: UiRect::axes(Val::Px(12.0), Val::Px(4.0)),
            ..default()
        }
    }

    fn add_observers(commands: &mut EntityCommands) {
        commands
            .observe(Self::on_enter)
            .observe(Self::on_leave)
            .observe(Self::on_down);
    }

    fn on_enter(
        trigger: Trigger<Pointer<Over>>,
        query_menu: Query<Entity, With<Menu>>,
        mut query: Query<(&mut BackgroundColor, Option<&MenuBarItem>), With<MenuItem>>,
        mut commands: Commands,
    ) {
        let Ok((mut background_color, is_menu_bar)) = query.get_mut(trigger.entity()) else {
            return;
        };

        *background_color = style::colors::HIGHLIGHT.into();

        let is_menu_opened = !query_menu.is_empty();
        let is_menu_bar = match is_menu_bar {
            Some(_) => true,
            None => false,
        };

        commands.trigger_targets(MenuItemEvent::Enter, [trigger.entity()]);

        if is_menu_bar {
            if is_menu_opened {
                commands.trigger_targets(MenuItemEvent::Pressed, [trigger.entity()]);
                Menu::close_all(&query_menu, &mut commands);
            }
        }
    }

    fn on_leave(
        trigger: Trigger<Pointer<Out>>,
        mut query: Query<&mut BackgroundColor, With<MenuItem>>,
        mut commands: Commands,
    ) {
        let Ok(mut background_color) = query.get_mut(trigger.entity()) else {
            return;
        };
    
        *background_color = style::colors::BACKGROUND.into();
        commands.trigger_targets(MenuItemEvent::Leave, [trigger.entity()]);
    }
    
    fn on_down(
        trigger: Trigger<Pointer<Down>>,
        query: Query<Entity, With<Menu>>,
        query_item: Query<&MenuBarItem>,
        mut commands: Commands,
    ) {
        let menu_opened = !query.is_empty();
        let is_owned_menu_bar = query_item.contains(trigger.entity());

        // Check if this menu item is owned by the menu bar. If so, only trigger if
        // no menu is open. If not part of the menu bar, always trigger.
        if is_owned_menu_bar {
            if !menu_opened {
                commands.trigger_targets(MenuItemEvent::Pressed, [trigger.entity()]);
            }
        } else {
            commands.trigger_targets(MenuItemEvent::Pressed, [trigger.entity()]);
        }

        Menu::close_all(&query, &mut commands);
    }
}

impl Default for MenuItem {
    fn default() -> Self {
        Self
    }
}

#[derive(Event, PartialEq, Eq)]
enum MenuEvent {
    CloseAll,
}

#[derive(Component)]
#[require(
    Node(Menu::node),
    BackgroundColor(|| style::colors::BACKGROUND),
)]
struct Menu {
    menu_item: Entity,
}

impl Menu {
    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Column,
            position_type: PositionType::Absolute,
            ..default()
        }
    }

    fn set_position(
        menu: &mut Node,
        menubar_item_transform: &GlobalTransform,
        menubar_item_computed: &ComputedNode,
    ) {
        let left_pad = menubar_item_computed.padding().left;
        let right_pad = menubar_item_computed.padding().right;
        let left = (menubar_item_transform.translation().x - left_pad - right_pad).max(0.0);
        menu.left = Val::Px(left);
        menu.top = Val::Px(menubar_item_computed.size().y);
    }

    fn on_added(
        trigger: Trigger<OnAdd, Menu>,
        item: Query<(&GlobalTransform, &ComputedNode, Option<&MenuBarItem>), With<MenuItem>>,
        mut menu: Query<(&mut Node, &Menu)>,
    ) {
        let Ok((mut node, menu)) = menu.get_mut(trigger.entity()) else {
            return;
        };

        let Ok((item_transform, item_computed, is_menu_bar)) = item.get(menu.menu_item) else {
            return;
        };

        match is_menu_bar {
            Some(_) => {
                Self::set_position(&mut node, item_transform, item_computed);
            },
            None => {},
        }
    }

    fn close_all(
        query: &Query<Entity, With<Menu>>,
        commands: &mut Commands,
    ) {
        for menu in query.iter() {
            commands.entity(menu).despawn_recursive();
        }
    }

    fn observe_close_all(
        trigger: Trigger<MenuEvent>,
        query: Query<Entity, With<Menu>>,
        mut commands: Commands,
    ) {
        if *trigger.event() == MenuEvent::CloseAll {
            Self::close_all(&query, &mut commands);
        }
    }
}
