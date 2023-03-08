#include "ShaderIncludes.hlsli"

//Define Constant Buffer
cbuffer ExternalData : register(b0) {
	float4 colorTint;
	float roughness;
	float3 cameraPosition;
	float3 ambient;

	Light lights[5];
}

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D SurfaceTextureSpecular : register(t1);
SamplerState BasicSampler : register(s0); // "s" registers for samplers

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	//Normalize the incoming normal
	input.normal = normalize(input.normal);

	//float3 CalculateLight(Light incomingLight, float3 normal, float4 surfaceColor, float3 ambient, float3 cameraPos, float3 worldPos, float roughness)

	float3 finalLight;
	float specular = SurfaceTextureSpecular.Sample(BasicSampler, input.uv).r;

	for (int i = 0; i < 5; i++) {
		switch (lights[i].Type) {
			case 0:
				finalLight += CalculateDirectionalLight(lights[i], input.normal, colorTint, ambient, cameraPosition, input.worldPosition, roughness, specular);
				break;

			case 1:
				finalLight += CalculatePointLight(lights[i], input.normal, colorTint, ambient, cameraPosition, input.worldPosition, roughness, specular);
				break;

			case 2:
				finalLight += float3(0, 0, 0);
				break;

			default:
				finalLight += float3(0, 0, 0);
				break;
		}
	}

	float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
	finalLight += surfaceColor;

	return float4(finalLight, 1);
}