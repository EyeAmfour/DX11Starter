#include "ShaderIncludes.hlsli"

TextureCube CubeMap : register(t0);
SamplerState SkySampler : register(s0);

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
	float3 cube = CubeMap.Sample(SkySampler, input.sampleDir).xyz;
	return float4(cube, 1);
}