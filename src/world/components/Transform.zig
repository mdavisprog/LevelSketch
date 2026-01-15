const core = @import("core");

const Mat = core.math.Mat;
const Rotation = core.math.Rotation;
const Vec = core.math.Vec;

const Self = @This();

translation: Vec = .zero,
rotation: Rotation = .zero,
scale: Vec = .zero,

pub fn toMatrix(self: Self) Mat {
    return .initTranslation(self.translation)
        .rotate(self.rotation)
        .scale(self.scale);
}
