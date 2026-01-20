const core = @import("core");

const Mat = core.math.Mat;
const Rotation = core.math.Rotation;
const Vec = core.math.Vec;

pub const Transform = struct {
    translation: Vec = .zero,
    rotation: Rotation = .zero,
    scale: Vec = .splat(1.0),

    pub fn toMatrix(self: Transform) Mat {
        return Mat.initTranslation(self.translation)
            .rotate(self.rotation)
            .scale(self.scale);
    }
};
