use bevy::{
    ecs::system::{
        IntoObserverSystem,
        SystemState,
    },
    prelude::*,
};
use crate::{
    controls::image::KeaImageNode,
    observers::KeaObservers,
    style,
};

///
/// KeaCheckboxState
///
/// The current state of the checkbox.
///
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum KeaCheckboxState {
    Unchecked,
    Checked,
}

///
/// KeaCheckbox
///
/// Control that can be toggled.
///
#[derive(Component)]
#[require(
    Node = Self::node(),
)]
pub struct KeaCheckbox {
    state: KeaCheckboxState,
    hot: bool,
}

impl KeaCheckbox {
    pub fn bundle<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>,
        label: &str,
    ) -> impl Bundle {
        Self::bundle_with_state(callback, label, KeaCheckboxState::Unchecked)
    }

    pub fn bundle_with_state<E: Event, B: Bundle, M>(
        callback: impl IntoObserverSystem<E, B, M>,
        label: &str,
        state: KeaCheckboxState,
    ) -> impl Bundle {(
        Self {
            state,
            hot: false,
        },
        KeaObservers::new(vec![
            Observer::new(on_over),
            Observer::new(on_out),
            Observer::new(on_pressed),
            Observer::new(on_click),
            Observer::new(callback),
        ]),
        children![
            (
                Box::bundle(),
            ),
            (
                Label::bundle(label),
            )
        ],
    )}

    pub fn state(&self) -> KeaCheckboxState {
        self.state
    }

    fn visibility(&self) -> Visibility {
        match self.state {
            KeaCheckboxState::Unchecked => Visibility::Hidden,
            KeaCheckboxState::Checked => Visibility::Visible,
        }
    }

    fn node() -> Node {
        Node {
            flex_direction: FlexDirection::Row,
            column_gap: Val::Px(10.0),
            align_items: AlignItems::Center,
            ..default()
        }
    }

    fn color(&self) -> Color {
        if self.hot {
            style::colors::PRESSED
        } else {
            match self.state {
                KeaCheckboxState::Unchecked => style::colors::BUTTON_BACKGROUND,
                KeaCheckboxState::Checked => style::colors::PRESSED,
            }
        }
    }
}

///
/// KeaCheckboxClicked
///
/// Event emitted when a checkbox is clicked. The state of the
/// of the checkbox is given with the event.
///
#[derive(Event)]
pub struct KeaCheckboxClicked {
    pub state: KeaCheckboxState,
}

/// Additional commands for checkboxes.
pub trait KeaCheckboxCommandsExt {
    fn kea_checkbox_set_state(&mut self, checkbox: Entity, state: KeaCheckboxState) -> &mut Self;
}

impl<'w, 's> KeaCheckboxCommandsExt for Commands<'w, 's> {
    fn kea_checkbox_set_state(&mut self, checkbox: Entity, state: KeaCheckboxState) -> &mut Self {
        self.queue(move |world: &mut World| set_check_state(world, checkbox, state));
        self
    }
}

///
/// Box
///
/// The visual box that displays the state of the checkbox.
///
#[derive(Component)]
#[require(
    Node = Self::node(),
    BorderRadius = style::properties::BORDER_RADIUS,
    BackgroundColor = style::colors::BUTTON_BACKGROUND,
    Pickable = Pickable::IGNORE,
)]
struct Box;

impl Box {
    const PATH: &str = "kea://icons/check.svg#image12x12";

    fn bundle() -> impl Bundle {(
        Self,
        children![(
            Check::bundle(Self::PATH),
        )]
    )}

    fn node() -> Node {
        Node {
            align_items: AlignItems::Center,
            justify_content: JustifyContent::Center,
            width: Val::Px(style::properties::FONT_SIZE * 1.5),
            height: Val::Px(style::properties::FONT_SIZE * 1.5),
            ..default()
        }
    }
}

///
/// Check
///
/// The image showing the 'check mark'.
///
#[derive(Component)]
#[require(
    Node = Self::node(),
    Visibility = Visibility::Hidden,
    Pickable = Pickable::IGNORE,
)]
struct Check;

impl Check {
    fn bundle(path: &str) -> impl Bundle {(
        Self,
        KeaImageNode(path.to_string()),
    )}

    fn node() -> Node {
        Node {
            ..default()
        }
    }
}

///
/// Label
///
/// Text that gives a label to the checkbox.
///
#[derive(Component)]
#[require(
    TextFont = TextFont::from_font_size(style::properties::FONT_SIZE),
    TextLayout = TextLayout::new_with_justify(JustifyText::Center),
    Pickable = Pickable::IGNORE,
)]
struct Label;

impl Label {
    fn bundle(text: &str) -> impl Bundle {(
        Self,
        Text::new(text),
    )}
}

pub(super) fn build(app: &mut App) {
    app
        .add_observer(on_add)
        .add_observer(on_released);
}

fn on_add(
    trigger: Trigger<OnAdd, Check>,
    checkboxes: Query<&KeaCheckbox>,
    children: Query<&ChildOf>,
    mut checks: Query<&mut Visibility, With<KeaImageNode>>,
) {
    let Ok(mut check) = checks.get_mut(trigger.target()) else {
        return;
    };

    for parent in children.iter_ancestors(trigger.target()) {
        let Ok(checkbox) = checkboxes.get(parent) else {
            continue;
        };

        *check = match checkbox.state() {
            KeaCheckboxState::Unchecked => Visibility::Hidden,
            KeaCheckboxState::Checked => Visibility::Visible,
        };

        break;
    }
}

fn set_box_color(
    color: Color,
    target: Entity,
    parents: &Query<&Children>,
    boxes: &mut Query<&mut BackgroundColor, With<Box>>,
) {
    for child in parents.iter_descendants(target) {
        if let Ok(mut box_color) = boxes.get_mut(child) {
            *box_color = color.into();
            break;
        }
    }
}

fn on_over(
    trigger: Trigger<Pointer<Over>>,
    parents: Query<&Children>,
    mut boxes: Query<&mut BackgroundColor, With<Box>>,
) {
    set_box_color(style::colors::HIGHLIGHT, trigger.target(), &parents, &mut boxes);
}

fn on_out(
    trigger: Trigger<Pointer<Out>>,
    parents: Query<&Children>,
    checkboxes: Query<&KeaCheckbox>,
    mut boxes: Query<&mut BackgroundColor, With<Box>>,
) {
    let Ok(checkbox) = checkboxes.get(trigger.target()) else {
        return;
    };

    set_box_color(checkbox.color(), trigger.target(), &parents, &mut boxes);
}

fn on_pressed(
    trigger: Trigger<Pointer<Pressed>>,
    parents: Query<&Children>,
    mut boxes: Query<&mut BackgroundColor, With<Box>>,
    mut checkboxes: Query<&mut KeaCheckbox>,
) {
    let Ok(mut checkbox) = checkboxes.get_mut(trigger.target()) else {
        return;
    };

    checkbox.hot = true;
    set_box_color(style::colors::PRESSED, trigger.target(), &parents, &mut boxes);
}

/// Global observer since the hot check box won't be hovered when this event is triggered.
fn on_released(
    _: Trigger<Pointer<Released>>,
    parents: Query<&Children>,
    checkboxes: Query<(Entity, &mut KeaCheckbox)>,
    mut boxes: Query<&mut BackgroundColor, With<Box>>,
) {
    for (entity, mut checkbox) in checkboxes {
        if checkbox.hot {
            checkbox.hot = false;
            set_box_color(checkbox.color(), entity, &parents, &mut boxes);
        }
    }
    
}

fn on_click(
    trigger: Trigger<Pointer<Click>>,
    parents: Query<&Children>,
    mut checkboxes: Query<&mut KeaCheckbox>,
    mut checks: Query<&mut Visibility, With<Check>>,
    mut commands: Commands,
) {
    let Ok(mut checkbox) = checkboxes.get_mut(trigger.target()) else {
        return;
    };

    checkbox.state = match checkbox.state {
        KeaCheckboxState::Unchecked => KeaCheckboxState::Checked,
        KeaCheckboxState::Checked => KeaCheckboxState::Unchecked,
    };

    for child in parents.iter_descendants(trigger.target()) {
        if let Ok(mut check) = checks.get_mut(child) {
            *check = checkbox.visibility();

            commands.trigger_targets(KeaCheckboxClicked {
                state: checkbox.state,
            }, trigger.target());

            break;
        }
    }
}

fn set_check_state(
    world: &mut World,
    checkbox_entity: Entity,
    state: KeaCheckboxState,
) {
    let mut system_state: SystemState<(
        Query<&Children>,
        Query<&mut KeaCheckbox>,
        Query<&mut Visibility, With<Check>>,
        Commands,
    )> = SystemState::new(world);

    let (
        parents,
        mut checkboxes,
        mut checks,
        mut commands,
    ) = system_state.get_mut(world);

    let Ok(mut checkbox) = checkboxes.get_mut(checkbox_entity) else {
        return;
    };

    checkbox.state = state;

    for child in parents.iter_descendants_depth_first(checkbox_entity) {
        let Ok(mut check) = checks.get_mut(child) else {
            continue;
        };

        *check = checkbox.visibility();

        commands
            .trigger_targets(KeaCheckboxClicked {
                state
            }, checkbox_entity);
        break;
    }

    system_state.apply(world);
}
