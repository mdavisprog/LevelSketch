# TrueType Package for Zig

This project started out as a port of
[stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h)
however it is independently maintained and improved upon by the open source
community.

Contributions welcome.

## Synopsis

```zig
const TrueType = @import("TrueType.zig");
const ttf = try TrueType.load(@embedFile("GoNotoCurrent-Regular.ttf"));
const example_string = "こんにちは!";
const scale = ttf.scaleForPixelHeight(20);
const stdout = std.io.getStdOut().writer();
var buffer: std.ArrayListUnmanaged(u8) = .empty;
defer buffer.deinit(gpa);
var it = std.unicode.Utf8View.initComptime(example_string).iterator();
while (it.nextCodepoint()) |codepoint| {
    if (ttf.codepointGlyphIndex(codepoint)) |glyph| {
        std.log.debug("0x{d}: {d}", .{ codepoint, glyph });
        buffer.clearRetainingCapacity();
        const dims = try ttf.glyphBitmap(gpa, &buffer, glyph, scale, scale);
        const pixels = buffer.items;
        for (0..dims.height) |j| {
            for (0..dims.width) |i| {
                try stdout.writeByte(" .:ioVM@"[pixels[j * dims.width + i] >> 5]);
            }
            try stdout.writeByte('\n');
        }
    } else {
        std.log.debug("0x{d}: none", .{codepoint});
    }
}
```

## Features and Limitations

* Codepoint to glyph lookup
* Glyph rendering to bitmap
* Kerning
* Font shaping and ligatures are not yet implemented.
* Untrusted font files are not supported.

## Roadmap

* eliminate TODOs
* eliminate heap allocation
* support more advanced text shaping like harfbuzz

## Why not use FreeType?

FreeType supports a lot more than just TrueType, making it bloated if your use
case is only TrueType fonts. 

FreeType is written in C. By having it written in Zig, we drop a dependency on
a C compiler and allow the code to be in the same compilation unit as the other
Zig code.

Healthy competition between open source projects.

## Why not use HarfBuzz?

HarfBuzz is written in C++, a programming language everyone agrees should be
wiped from the face of the Earth. My personal code of conduct forbids me from
adding libc++ as a runtime dependency to any of my projects.

Healthy competition between open source projects.
