#include "ShaderIncludes.hlsli"

//Define Constant Buffer
cbuffer ExternalData : register(b0) {
	float4 colorTint;
	float roughness;
	float3 cameraPosition;
	float3 ambient;

	Light lights[5];
}

Texture2D Albedo : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
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
	//input.normal = normalize(input.tangent);

	//Handle normal mapping
	float3 unpackedNormal = normalize(NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1);
	//input.normal = normalFromMap;

	//Rotate the normal map to convert from TANGENT to WORLD space
	//Ensure we "orthonormalize" the tangent again
	float3 N = normalize(input.normal); // Must be normalized here or before
	float3 T = normalize(input.tangent); // Must be normalized here or before
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	//Multiply the normal map vector by the TBN
	input.normal = mul(unpackedNormal, TBN);

	float3 finalLight;
	//float specular = SurfaceTextureSpecular.Sample(BasicSampler, input.uv).r;
	float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
	float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

	//Un-correct surface texture sample to adjust for gamma correction
	float3 albedo = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);

	// Specular color determination -----------------
	// Assume albedo texture is actually holding specular color where metalness == 1
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
	float3 specularColor = lerp(F0_NON_METAL, albedo.rgb, metalness);

	//Ensure ambient is 0 because it conflicts with PBR
	/*ambient = float3(0, 0, 0);*/

	for (int i = 0; i < 5; i++) {
		switch (lights[i].Type) {
			case 0:
				//float3 CalculateDirectionalLight(Light incomingLight, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness, float metalness, float3 specularColor) {
				finalLight += CalculateDirectionalLight(lights[i], input.normal, colorTint, cameraPosition, input.worldPosition, roughness, metalness, specularColor);
				break;

			case 1:
				finalLight += CalculatePointLight(lights[i], input.normal, colorTint, ambient, cameraPosition, input.worldPosition, roughness, specularColor);
				break;

			case 2:
				finalLight += float3(0, 0, 0);
				break;

			default:
				finalLight += float3(0, 0, 0);
				break;
		}
	}

	finalLight *= albedo;

	return float4(pow(finalLight, 1.0f / 2.2f), 1);
}