use bevy::prelude::*;

#[derive(Component, Default)]
pub struct KeaNodeOverrides {
    pub display: Option<Display>,
    pub box_sizing: Option<BoxSizing>,
    pub position_type: Option<PositionType>,
    pub overflow: Option<Overflow>,
    pub overflow_clip_margin: Option<OverflowClipMargin>,
    pub left: Option<Val>,
    pub right: Option<Val>,
    pub top: Option<Val>,
    pub bottom: Option<Val>,
    pub width: Option<Val>,
    pub height: Option<Val>,
    pub min_width: Option<Val>,
    pub min_height: Option<Val>,
    pub max_width: Option<Val>,
    pub max_height: Option<Val>,
    pub aspect_ratio: Option<f32>,
    pub align_items: Option<AlignItems>,
    pub justify_items: Option<JustifyItems>,
    pub align_self: Option<AlignSelf>,
    pub justify_self: Option<JustifySelf>,
    pub align_content: Option<AlignContent>,
    pub justify_content: Option<JustifyContent>,
    pub margin: Option<UiRect>,
    pub padding: Option<UiRect>,
    pub border: Option<UiRect>,
    pub flex_direction: Option<FlexDirection>,
    pub flex_wrap: Option<FlexWrap>,
    pub flex_grow: Option<f32>,
    pub flex_shrink: Option<f32>,
    pub flex_basis: Option<Val>,
    pub row_gap: Option<Val>,
    pub column_gap: Option<Val>,
    pub grid_auto_flow: Option<GridAutoFlow>,
    pub grid_template_rows: Option<Vec<RepeatedGridTrack>>,
    pub grid_template_columns: Option<Vec<RepeatedGridTrack>>,
    pub grid_auto_rows: Option<Vec<GridTrack>>,
    pub grid_auto_columns: Option<Vec<GridTrack>>,
    pub grid_row: Option<GridPlacement>,
    pub grid_column: Option<GridPlacement>,
}

pub(super) fn build(app: &mut App) {
    app.add_observer(on_add);
}

fn assign<T>(src: Option<T>, dest: &mut T) {
    match src {
        Some(value) => {
            *dest = value;
        },
        _ => {},
    }
}

fn on_add(
    trigger: Trigger<OnAdd, KeaNodeOverrides>,
    mut node_overrides: Query<&mut KeaNodeOverrides>,
    mut nodes: Query<&mut Node>,
    mut commands: Commands,
) {
    let Ok(mut node_override) = node_overrides.get_mut(trigger.target()) else {
        return;
    };

    let Ok(mut node) = nodes.get_mut(trigger.target()) else {
        return;
    };

    assign(node_override.display, &mut node.display);
    assign(node_override.box_sizing, &mut node.box_sizing);
    assign(node_override.position_type, &mut node.position_type);
    assign(node_override.overflow, &mut node.overflow);
    assign(node_override.overflow_clip_margin, &mut node.overflow_clip_margin);
    assign(node_override.left, &mut node.left);
    assign(node_override.right, &mut node.right);
    assign(node_override.top, &mut node.top);
    assign(node_override.bottom, &mut node.bottom);
    assign(node_override.width, &mut node.width);
    assign(node_override.height, &mut node.height);
    assign(node_override.min_width, &mut node.min_width);
    assign(node_override.min_height, &mut node.min_height);
    assign(node_override.max_width, &mut node.max_width);
    assign(node_override.max_height, &mut node.max_height);
    assign(node_override.align_items, &mut node.align_items);
    assign(node_override.justify_items, &mut node.justify_items);
    assign(node_override.align_self, &mut node.align_self);
    assign(node_override.justify_self, &mut node.justify_self);
    assign(node_override.align_content, &mut node.align_content);
    assign(node_override.justify_content, &mut node.justify_content);
    assign(node_override.margin, &mut node.margin);
    assign(node_override.padding, &mut node.padding);
    assign(node_override.border, &mut node.border);
    assign(node_override.flex_direction, &mut node.flex_direction);
    assign(node_override.flex_wrap, &mut node.flex_wrap);
    assign(node_override.flex_grow, &mut node.flex_grow);
    assign(node_override.flex_shrink, &mut node.flex_shrink);
    assign(node_override.flex_basis, &mut node.flex_basis);
    assign(node_override.row_gap, &mut node.row_gap);
    assign(node_override.column_gap, &mut node.column_gap);
    assign(node_override.grid_auto_flow, &mut node.grid_auto_flow);
    assign(node_override.grid_template_rows.take(), &mut node.grid_template_rows);
    assign(node_override.grid_template_columns.take(), &mut node.grid_template_columns);
    assign(node_override.grid_auto_rows.take(), &mut node.grid_auto_rows);
    assign(node_override.grid_auto_columns.take(), &mut node.grid_auto_columns);
    assign(node_override.grid_row, &mut node.grid_row);
    assign(node_override.grid_column, &mut node.grid_column);

    commands
        .entity(trigger.target())
        .remove::<KeaNodeOverrides>();
}
