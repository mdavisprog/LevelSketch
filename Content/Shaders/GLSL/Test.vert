/**

MIT License

Copyright (c) 2024 Mitchell Davis <mdavisprog@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#version 450

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UVs;
layout(location = 2) in vec4 Color;

layout(location = 0) out vec4 OutColor;
layout(location = 1) out vec2 OutUVs;

layout(set = 0, binding = 0) uniform UniformObject
{
    mat4 Model;
    mat4 View;
    mat4 Projection;
    mat4 Orthographic;
} Uniforms;

void main()
{
    gl_Position = Uniforms.Projection * Uniforms.View * Uniforms.Model * vec4(Position, 1.0);
    OutColor = Color;
    OutUVs = UVs;
}