use bevy::{
    ecs::system::{
        IntoObserverSystem,
        SystemState,
    },
    prelude::*,
};
use crate::{
    observers::KeaObservers,
    ready::{
        KeaOnReady,
        KeaOnReadyComponent,
    },
    style,
};
use super::{
    button::{
        KeaButton,
        KeaButtonClick,
    },
    image::KeaImageNode,
    label::KeaLabel,
    list::{
        KeaList,
        KeaListBehavior,
        KeaListLabelItems,
        KeaListSelect,
    },
    popup::{
        KeaPopupCommands,
        KeaPopupPosition,
        KeaPopupSize,
        KeaPopupState,
    },
};

/// Determines if the KeaDropdown component should manage its own popup or a custom one
/// should be provided. Refer to KeaDropdownCustom on more information.
pub enum KeaDropdownBehavior {
    Default,
    Custom,
}

#[derive(Component)]
pub struct KeaDropdown {
    selected: usize,
    behavior: KeaDropdownBehavior,
    should_open_popup: bool,
}

impl KeaDropdown {
    pub fn bundle() -> impl Bundle {(
        Self::internal_bundle(KeaDropdownBehavior::Default),
    )}

    pub fn bundle_with_items(items: Vec<String>) -> impl Bundle {(
        Self::internal_bundle(KeaDropdownBehavior::Default),
        KeaDropdownItems(items),
    )}

    pub fn bundle_custom<E: Event, B: Bundle, M>(
        on_custom: impl IntoObserverSystem<E, B, M>,
    ) -> impl Bundle {(
        Self::internal_bundle(KeaDropdownBehavior::Custom),
        KeaObservers::<DropdownCustom>::new(vec![
            Observer::new(on_custom),
        ]),
    )}

    pub fn selected(&self) -> usize {
        self.selected
    }

    fn internal_bundle(behavior: KeaDropdownBehavior) -> impl Bundle {(
        Self {
            selected: 0,
            behavior,
            should_open_popup: false,
        },
        KeaButton::bundle(on_click),
        KeaObservers::<Self>::new(vec![
            Observer::new(on_pressed),
        ]),
        children![(
            Node {
                width: Val::Percent(100.0),
                ..default()
            },
            Pickable::IGNORE,
            children![
                (
                    Node {
                        position_type: PositionType::Absolute,
                        width: Val::Percent(100.0),
                        justify_content: JustifyContent::Center,
                        ..default()
                    },
                    Pickable::IGNORE,
                    children![(
                        KeaLabel::bundle(""),
                        Pickable::IGNORE,
                        DropdownLabel,
                    )]
                ),
                (
                    Node {
                        width: Val::Percent(100.0),
                        justify_content: JustifyContent::End,
                        ..default()
                    },
                    Pickable::IGNORE,
                    children![(
                        KeaImageNode(format!(
                            "kea://icons/expander.svg#image{size}x{size}",
                            size = style::properties::FONT_SIZE),
                        ),
                        Pickable::IGNORE,
                    )]
                ),
            ],
        )],
    )}
}

pub trait KeaDropdownCommands {
    fn kea_dropdown_set_label(&mut self, dropdown: Entity, label: String) -> &mut Self;
}

impl<'w, 's> KeaDropdownCommands for Commands<'w, 's> {
    fn kea_dropdown_set_label(&mut self, dropdown: Entity, label: String) -> &mut Self {
        self.queue(move |world: &mut World| {
            let mut system_state: SystemState<(
                Query<&KeaDropdown>,
                Query<&Children>,
                Query<&mut Text, With<DropdownLabel>>,
            )> = SystemState::new(world);

            let (
                dropdowns,
                parents,
                mut texts,
            ) = system_state.get_mut(world);

            if !dropdowns.contains(dropdown) {
                warn!("Entity {dropdown} is not a KeaDropdown.");
                return;
            }

            for child in parents.iter_descendants(dropdown) {
                let Ok(mut text) = texts.get_mut(child) else {
                    continue;
                };

                text.0 = label;
                break;
            }

            system_state.apply(world);
        });
        self
    }
}

#[derive(Component)]
#[require(
    KeaOnReadyComponent,
    KeaObservers<Self> = Self::observers(),
)]
pub struct KeaDropdownItems(pub Vec<String>);

impl KeaDropdownItems {
    fn observers() -> KeaObservers<Self> {
        KeaObservers::new(vec![
            Observer::new(on_ready),
        ])
    }
}

#[derive(Event)]
pub struct KeaDropdownSelect {
    pub index: usize,
}

#[derive(Event)]
pub struct KeaDropdownCustom {
    /// Fill in components for this entity in the event listener. The popup is already open.
    pub entity: Entity,
}

#[derive(Component)]
struct DropdownLabel;

/// Used for setting up observers.
struct DropdownCustom;

#[derive(Component)]
#[require(
    Node = Self::node(),
    BackgroundColor(style::colors::BACKGROUND),
    KeaObservers<Self> = Self::observers(),
)]
struct DropdownPopup {
    dropdown: Entity,
}

impl DropdownPopup {
    fn bundle(
        dropdown: Entity,
        items: Vec<String>,
    ) -> impl Bundle {(
        Self {
            dropdown,
        },
        KeaList::new(KeaListBehavior::Select),
        KeaListLabelItems(items),
    )}

    fn node() -> Node {
        Node {
            width: Val::Percent(100.0),
            height: Val::Percent(100.0),
            flex_direction: FlexDirection::Column,
            ..default()
        }
    }

    fn observers() -> KeaObservers<Self> {
        KeaObservers::new(vec![
            Observer::new(on_select),
        ])
    }
}

fn on_ready(
    trigger: Trigger<KeaOnReady>,
    dropdowns: Query<(&KeaDropdown, &KeaDropdownItems)>,
    mut commands: Commands,
) {
    let Ok((dropdown, items)) = dropdowns.get(trigger.target()) else {
        return;
    };

    let label = match items.0.get(dropdown.selected) {
        Some(value) => value.clone(),
        None => format!(""),
    };

    commands.kea_dropdown_set_label(trigger.target(), label);
}

fn on_pressed(
    trigger: Trigger<Pointer<Pressed>>,
    popup: Res<KeaPopupState>,
    mut dropdowns: Query<&mut KeaDropdown>,
) {
    let Ok(mut dropdown) = dropdowns.get_mut(trigger.target()) else {
        return;
    };

    dropdown.should_open_popup = !popup.is_open();
}

fn on_click(
    trigger: Trigger<KeaButtonClick>,
    nodes: Query<(&ComputedNode, &GlobalTransform)>,
    dropdowns: Query<(&KeaDropdown, Option<&KeaDropdownItems>)>,
    mut commands: Commands,
) {
    let Ok((node, transform)) = nodes.get(trigger.target()) else {
        panic!("Failed to get ComputedNode and GlobalTransform components for KeaDropdown.");
    };

    let Ok((dropdown, items)) = dropdowns.get(trigger.target()) else {
        return;
    };

    if !dropdown.should_open_popup {
        return;
    }

    let bounds = Rect::from_center_size(
        transform.translation().truncate(),
        node.size,
    );

    let position = KeaPopupPosition::At(Vec2::new(bounds.min.x, bounds.max.y).as_ivec2());
    let size = KeaPopupSize::Auto;
    let root_node = Node {
        width: Val::Px(bounds.width()),
        ..default()
    };

    match dropdown.behavior {
        KeaDropdownBehavior::Custom => {
            let entity = commands.kea_popup_open(
                root_node,
                position,
                size
            ).id();

            commands.trigger_targets(KeaDropdownCustom {
                entity,
            }, trigger.target());
        },
        KeaDropdownBehavior::Default => {
            let Some(items) = items else {
                return;
            };

            commands.kea_popup_open(
                (
                    root_node,
                    children![(
                        DropdownPopup::bundle(
                            trigger.target(),
                            items.0.clone(),
                        ),
                    )],
                ),
                position,
                size,
            );
        },
    }
}

fn on_select(
    trigger: Trigger<KeaListSelect>,
    dropdown_popups: Query<&DropdownPopup>,
    dropdown_items: Query<&KeaDropdownItems>,
    mut commands: Commands,
) {
    let Ok(popup) = dropdown_popups.get(trigger.target()) else {
        panic!("Failed to get DropdownPopup for entity {}.", trigger.target());
    };

    let Ok(items) = dropdown_items.get(popup.dropdown) else {
        warn!("No KeaDropdownItems component found for KeaDropdown {}.", popup.dropdown);
        return;
    };

    let event = trigger.event();
    let label = match items.0.get(event.index) {
        Some(value) => value.clone(),
        None => format!(""),
    };

    commands
        .kea_dropdown_set_label(popup.dropdown, label)
        .kea_popup_close()
        .trigger_targets(KeaDropdownSelect {
            index: event.index,
        }, popup.dropdown);
}
