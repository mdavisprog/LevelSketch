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

struct RasterizerData
{
    float4 Position [[position]];
    float2 UV;
    float4 Color;
};

struct Vertex2
{
    float2 Position [[ attribute(0) ]];
    float2 UV [[ attribute(1) ]];
    uchar4 Color [[ attribute(2) ]];
};

struct Uniforms
{
    metal::float4x4 Model;
    metal::float4x4 View;
    metal::float4x4 Perspective;
    metal::float4x4 Orthographic;
};

vertex RasterizerData Main(Vertex2 Vertex [[ stage_in ]], constant Uniforms& Uniforms_ [[ buffer(1) ]])
{
    RasterizerData Out;

    Out.Position = Uniforms_.Orthographic * float4(Vertex.Position, 0.0, 1.0);
    Out.UV = Vertex.UV;
    Out.Color = float4(Vertex.Color) / float4(255.0);

    return Out;
}
