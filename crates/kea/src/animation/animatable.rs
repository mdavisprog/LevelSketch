use bevy::prelude::*;

#[reflect_trait]
pub trait KeaAnimatable {
    fn animate(
        &self,
        target: &dyn PartialReflect,
        time: f32,
    ) -> Result<Box<dyn PartialReflect>, String>;
}

impl KeaAnimatable for Val {
    fn animate(
        &self,
        target: &dyn PartialReflect,
        time: f32,
    ) -> Result<Box<dyn PartialReflect>, String> {
        let Some(target) = target.try_downcast_ref::<Val>() else {
            return Err(format!("Failed to downcast to Val type."));
        };

        // Converts the given Val enum to the internal f32 data.
        fn to_f32(val: &Val) -> f32 {
            match val {
                Val::Auto => 0.0,
                Val::Percent(result) => *result,
                Val::Px(result) => *result,
                Val::VMax(result) => *result,
                Val::VMin(result) => *result,
                Val::Vh(result) => *result,
                Val::Vw(result) => *result,
            }
        }

        let lhs = to_f32(self);
        let rhs = to_f32(target);
        let sample = lhs.lerp(rhs, time);

        // Output should match target variant.
        let result = match target {
            Val::Auto => Val::Auto,
            Val::Percent(_) => Val::Percent(sample),
            Val::Px(_) => Val::Px(sample),
            Val::VMax(_) => Val::VMax(sample),
            Val::VMin(_) => Val::VMin(sample),
            Val::Vh(_) => Val::Vh(sample),
            Val::Vw(_) => Val::Vw(sample),
        };

        Ok(Box::new(result))
    }
}

impl KeaAnimatable for Quat {
    fn animate(
        &self,
        target: &dyn PartialReflect,
        time: f32,
    ) -> Result<Box<dyn PartialReflect>, String> {
        let Some(target) = target.try_downcast_ref::<Quat>() else {
            return Err(format!("Failed to downcast to Quat type."));
        };

        Ok(Box::new(self.lerp(*target, time)))
    }
}

impl KeaAnimatable for f32 {
    fn animate(
        &self,
        target: &dyn PartialReflect,
        time: f32,
    ) -> Result<Box<dyn PartialReflect>, String> {
        let Some(target) = target.try_downcast_ref::<f32>() else {
            return Err(format!("Failed to downcast to f32 target."));
        };

        Ok(Box::new(self.lerp(*target, time)))
    }
}
