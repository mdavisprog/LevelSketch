use bevy::prelude::*;

///
/// KeaPosition
///
/// Component that positions this Node when added. This
/// component is removed after positioning.
///
#[derive(Component, Clone, Copy)]
pub struct KeaPosition(pub Vec3);

pub(super) fn build(app: &mut App) {
    app.add_observer(on_add);
}

fn on_add(
    trigger: Trigger<OnAdd, KeaPosition>,
    positions: Query<&KeaPosition>,
    mut nodes: Query<&mut Node>,
    mut commands: Commands,
) {
    let entity = trigger.target();

    let Ok(position) = positions.get(entity) else {
        return;
    };

    let Ok(mut node) = nodes.get_mut(entity) else {
        return;
    };

    node.left = Val::Px(position.0.x);
    node.top = Val::Px(position.0.y);

    commands
        .entity(entity)
        .remove::<KeaPosition>();
}
