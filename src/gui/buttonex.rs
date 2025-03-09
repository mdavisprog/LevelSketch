use bevy::ecs::system::IntoObserverSystem;
use bevy::picking::focus::HoverMap;
use bevy::prelude::*;
use super::style;

#[derive(Event)]
pub struct OnClick;

///
/// LevelSketch's own custom button.
/// 
#[derive(Component)]
#[require(
    Node(ButtonEx::node),
    BackgroundColor(|| style::colors::BACKGROUND),
)]
pub struct ButtonEx;

impl ButtonEx {
    pub fn initialize(commands: &mut Commands) {
        commands.add_observer(Self::on_up);
    }

    pub fn create_label<E: Event, B: Bundle, M>(
        commands: &mut ChildBuilder,
        label: &str,
        callback: impl IntoObserverSystem<E, B, M>,
        components: impl Bundle,
    ) -> Entity {
        let mut entity = commands.spawn((
            Self,
            components,
        ));

        entity.with_children(|parent| {
            parent.spawn((
                Text::new(label),
                TextFont::from_font_size(12.0),
                PickingBehavior::IGNORE,
            ));
        });

        Self::add_observers(&mut entity, callback);

        entity.id()
    }

    pub fn create_image<E: Event, B: Bundle, M>(
        commands: &mut ChildBuilder,
        image: Handle<Image>,
        callback: impl IntoObserverSystem<E, B, M>,
        components: impl Bundle,
    ) -> Entity {
        let mut entity = commands.spawn((
            Self,
            components,
        ));

        entity.with_children(|parent| {
            parent.spawn((
                ImageNode::new(image),
                PickingBehavior::IGNORE,
            ));
        });

        Self::add_observers(&mut entity, callback);

        entity.id()
    }

    fn add_observers<E: Event, B: Bundle, M>(
        entity: &mut EntityCommands,
        callback: impl IntoObserverSystem<E, B, M>,
    ) {
        entity
            .observe(Self::on_over)
            .observe(Self::on_out)
            .observe(Self::on_down)
            .observe(Self::on_click)
            .observe(callback);
    }

    fn node() -> Node {
        Node {
            padding: style::properties::BUTTON_PADDING,
            ..default()
        }
    }

    fn on_over(
        trigger: Trigger<Pointer<Over>>,
        state: Res<State>,
        mut colors: Query<&mut BackgroundColor, With<Self>>,
    ) {
        let Ok(mut color) = colors.get_mut(trigger.target) else {
            return;
        };

        if state.hot_button.is_none() {
            *color = style::colors::HIGHLIGHT.into();
        }
    }

    fn on_out(
        trigger: Trigger<Pointer<Out>>,
        state: Res<State>,
        mut colors: Query<&mut BackgroundColor, With<ButtonEx>>,
    ) {
        let Ok(mut color) = colors.get_mut(trigger.target) else {
            return;
        };

        let change_color = match state.hot_button {
            Some(hot) => {
                if hot != trigger.target {
                    true
                } else {
                    false
                }
            }
            None => true
        };

        if change_color {
            *color = style::colors::BACKGROUND.into();
        }
    }

    fn on_down(
        trigger: Trigger<Pointer<Down>>,
        mut colors: Query<&mut BackgroundColor, With<ButtonEx>>,
        mut state: ResMut<State>,
    ) {
        let Ok(mut color) = colors.get_mut(trigger.target) else {
            return;
        };

        *color = style::colors::PRESSED.into();
        state.hot_button = Some(trigger.target);
    }

    ///
    /// This is observed only globally. Need to detect cases where 
    /// the mouse is up, but no longer on the hot button.
    /// 
    fn on_up(
        trigger: Trigger<Pointer<Up>>,
        hovers: Res<HoverMap>,
        mut colors: Query<&mut BackgroundColor, With<ButtonEx>>,
        mut state: ResMut<State>,
    ) {
        match state.hot_button {
            Some(hot) => {
                if let Ok(mut color) = colors.get_mut(hot) {
                    if hot == trigger.target {
                        *color = style::colors::HIGHLIGHT.into();
                    } else {
                        *color = style::colors::BACKGROUND.into();
                    }
                }
            },
            None => {}
        }

        state.hot_button = None;

        let hover_map = &hovers.0[&trigger.pointer_id];
        for (entity, _) in hover_map {
            if let Ok(mut color) = colors.get_mut(*entity) {
                *color = style::colors::HIGHLIGHT.into();
                break;
            }
        }
    }

    fn on_click(
        trigger: Trigger<Pointer<Click>>,
        mut commands: Commands,
    ) {
        commands.trigger_targets(OnClick, [trigger.target]);
    }
}

#[derive(Resource)]
pub struct State {
    hot_button: Option<Entity>,
}

impl Default for State {
    fn default() -> Self {
        Self {
            hot_button: None,
        }
    }
}
