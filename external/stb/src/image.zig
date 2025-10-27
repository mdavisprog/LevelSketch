pub const Image = struct {
    data: [*]u8,
    width: u16,
    height: u16,
    channels: u8,

    pub fn size(self: Image) usize {
        const width: usize = @intCast(self.width);
        const height: usize = @intCast(self.height);
        const channels: usize = @intCast(self.channels);
        return width * height * channels;
    }
};

pub const Error = error{
    FailedLoadFromMemory,
};

pub fn load_from_memory(buffer: []const u8) !Image {
    var x: c_int = 0;
    var y: c_int = 0;
    var channels: c_int = 0;

    const data = stbi_load_from_memory(
        buffer.ptr,
        @intCast(buffer.len),
        &x,
        &y,
        &channels,
        0,
    );

    if (data == null) {
        return Error.FailedLoadFromMemory;
    }

    return Image{
        .data = data.?,
        .width = @intCast(x),
        .height = @intCast(y),
        .channels = @intCast(channels),
    };
}

pub fn free(image: Image) void {
    stbi_image_free(image.data);
}

extern fn stbi_load_from_memory(
    buffer: [*]const u8,
    len: c_int,
    x: *c_int,
    y: *c_int,
    channels_in_file: *c_int,
    desired_channels: c_int,
) ?[*]u8;

extern fn stbi_image_free(buffer: [*]const u8) void;
