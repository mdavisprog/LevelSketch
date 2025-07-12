use bevy::prelude::*;
use std::{
    any::TypeId,
    sync::Arc,
};

#[derive(Component)]
pub struct KeaAnimation {
    pub(crate) clips: Vec<KeaAnimationClip>,
}

impl KeaAnimation {
    pub fn new() -> Self {
        Self {
            clips: Vec::new(),
        }
    }

    pub fn new_with_clips(clips: Vec<KeaAnimationClip>) -> Self {
        Self {
            clips,
        }
    }

    pub fn add_clip(mut self, clip: KeaAnimationClip) -> Self {
        self.clips.push(clip);
        self
    }
}

#[derive(Clone)]
pub struct KeaAnimationClip {
    pub(crate) component_type: TypeId,
    pub(crate) field_path: String,
    pub(crate) value: Arc<dyn PartialReflect>,
    pub(crate) duration: f32,
    pub(crate) elapsed: f32,
    pub(crate) ease: EaseFunction,
}

impl KeaAnimationClip {
    pub fn new<T: Component>(
        field: &str,
        value: impl PartialReflect,
        duration: f32,
    ) -> Self {
        Self::new_with_ease::<T>(field, value, duration, EaseFunction::Linear)
    }

    pub fn new_with_ease<T: Component>(
        field: &str,
        value: impl PartialReflect,
        duration: f32,
        ease: EaseFunction,
    ) -> Self {
        Self {
            component_type: TypeId::of::<T>(),
            field_path: field.to_string(),
            value: Arc::new(value),
            duration,
            elapsed: 0.0,
            ease,
        }
    }

    pub fn set_ease(mut self, ease: EaseFunction) -> Self {
        self.ease = ease;
        self
    }

    pub fn progress(&self) -> f32 {
        self.elapsed / self.duration
    }

    pub fn get_time(&self) -> f32 {
        self.ease.sample_clamped(self.progress())
    }

    pub fn finish(&mut self) -> &mut Self {
        self.elapsed = self.duration;
        self
    }

    pub fn is_complete(&self) -> bool {
        self.progress() >= 1.0
    }
}
