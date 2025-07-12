use bevy::{
    ecs::system::SystemState,
    prelude::*,
};
use super::{
    animatable::ReflectKeaAnimatable,
    component::{
        KeaAnimation,
        KeaAnimationClip,
    },
    events::KeaAnimationComplete,
};

pub(super) fn build(app: &mut App) {
    app
        .add_systems(Startup, setup)
        .add_systems(PostUpdate, update);
}

fn setup(app_type_registry: Res<AppTypeRegistry>) {
    let mut type_registry = app_type_registry.write();
    type_registry.register_type_data::<Val, ReflectKeaAnimatable>();
    type_registry.register_type_data::<Quat, ReflectKeaAnimatable>();
    type_registry.register_type_data::<f32, ReflectKeaAnimatable>();
}

fn update(
    world: &mut World,
) {
    struct AnimationRef {
        entity: Entity,
        clips: Vec<KeaAnimationClip>,
    }

    let mut anim_refs: Vec<AnimationRef> = Vec::new();
    {
        let mut system_state: SystemState<(
            Res<Time>,
            Query<(Entity, &mut KeaAnimation)>,
        )> = SystemState::new(world);

        let (
            time,
            animations,
        ) = system_state.get_mut(world);

        // Advance the animation and grab a copy of each clip per entity.
        for (entity, mut animation) in animations {
            for clip in &mut animation.clips {
                clip.elapsed += time.delta_secs();
            }

            anim_refs.push(AnimationRef {
                entity,
                clips: animation.clips.clone(),
            });
        }

        system_state.apply(world);
    }

    // Get a clone of the app's registry to prevent holding a borrow of the world.
    let app_type_registry = world.resource::<AppTypeRegistry>().clone();

    // Animate the requested field and apply the change.
    for anim_ref in &mut anim_refs {
        for clip in &mut anim_ref.clips {
            if let Err(error) = update_property(&clip, anim_ref.entity, world, &app_type_registry) {
                warn!("Error: {error}");
                clip.finish();
            }
        }
    }

    // Remove expired animation components.
    let mut commands = world.commands();
    for anim_ref in &anim_refs {
        for clip in &anim_ref.clips {
            if clip.is_complete() {
                commands
                    .entity(anim_ref.entity)
                    .remove::<KeaAnimation>()
                    .trigger(KeaAnimationComplete {
                        component_type: clip.component_type,
                        field: clip.field_path.clone(),
                    });
            }
        }
    }
    world.flush();
}

fn update_property(
    animation: &KeaAnimationClip,
    entity: Entity,
    world: &mut World,
    app_type_registry: &AppTypeRegistry,
) -> Result<(), String> {
    let type_registry = app_type_registry.read();

    let Ok(mut reflected) = world.get_reflect_mut(entity, animation.component_type) else {
        return Err(format!("Failed to find component {:?}.", animation.component_type));
    };

    let Ok(value) = reflected.reflect_path_mut(animation.field_path.as_str()) else {
        return Err(format!("Failed to find the field '{}'.", animation.field_path));
    };

    let Some(reflect_value) = value.try_as_reflect() else {
        return Err(format!("Failed to get reflected value."));
    };

    let reflect_type_path = reflect_value.reflect_type_path();
    let Some(registration) = type_registry.get_with_type_path(reflect_type_path) else {
        return Err(format!("Failed to get type registration for value '{}'.", reflect_type_path));
    };

    let Some(reflect_animatable) = registration.data::<ReflectKeaAnimatable>() else {
        return Err(format!("Failed to get ReflectKeaAnimatable data."));
    };

    let Some(animatable) = reflect_animatable.get(reflect_value) else {
        return Err(format!("Failed to get animatable trait."));
    };

    let time = animation.get_time();
    let result = match animatable.animate(&(*animation.value), time) {
        Ok(result) => result,
        Err(error) => return Err(error),
    };

    value.apply(&(*result));

    Ok(())
}
