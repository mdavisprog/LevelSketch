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
    float4 ClipSpacePosition [[position]];
    float2 UV;
    float4 Color;
};

struct Vertex3
{
    float3 Position [[ attribute(0) ]];
    float2 UV [[ attribute(1) ]];
    float4 Color [[ attribute(2) ]];
};

vertex RasterizerData VertexMain(Vertex3 Vertex [[ stage_in ]])
{
    RasterizerData Out;

    Out.ClipSpacePosition = float4(Vertex.Position, 1.0);
    Out.UV = Vertex.UV;
    Out.Color = Vertex.Color;

    return Out;
}

fragment float4 PixelMain(RasterizerData Data [[stage_in]])
{
    return Data.Color;
}
