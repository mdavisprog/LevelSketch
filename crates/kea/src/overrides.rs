use bevy::prelude::*;

#[derive(Component, Default)]
pub(crate) struct NodeOverrides {
    pub display: Option<Display>,
    pub position_type: Option<PositionType>,
    pub align_self: Option<AlignSelf>,
    pub padding: Option<UiRect>,
    pub flex_grow: Option<f32>,
}

pub(super) fn build(app: &mut App) {
    app.add_observer(on_add);
}

fn assign<T: Clone + Copy>(src: &Option<T>, dest: &mut T) {
    match src {
        Some(value) => {
            *dest = *value;
        },
        _ => {},
    }
}

fn on_add(
    trigger: Trigger<OnAdd, NodeOverrides>,
    node_overrides: Query<&NodeOverrides>,
    mut nodes: Query<&mut Node>,
    mut commands: Commands,
) {
    let Ok(node_override) = node_overrides.get(trigger.target()) else {
        return;
    };

    let Ok(mut node) = nodes.get_mut(trigger.target()) else {
        return;
    };

    assign(&node_override.display, &mut node.display);
    assign(&node_override.position_type, &mut node.position_type);
    assign(&node_override.align_self, &mut node.align_self);
    assign(&node_override.padding, &mut node.padding);
    assign(&node_override.flex_grow, &mut node.flex_grow);

    commands
        .entity(trigger.target())
        .remove::<NodeOverrides>();
}
