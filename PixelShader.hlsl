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
Texture2D ShadowMap : register(t4);
SamplerState BasicSampler : register(s0); // "s" registers for samplers
SamplerComparisonState ShadowSampler : register(s1);

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
	float3 finalLight;

	// Perform the perspective divide (divide by W) ourselves
	input.shadowMapPos /= input.shadowMapPos.w;

	// Convert the normalized device coordinates to UVs for sampling
	float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
	shadowUV.y = 1 - shadowUV.y; // Flip the Y

	// Grab the distances we need: light-to-pixel and closest-surface
	float distToLight = input.shadowMapPos.z;

	// Get a ratio of comparison results using SampleCmpLevelZero()
	float shadowAmount = ShadowMap.SampleCmpLevelZero(
		ShadowSampler,
		shadowUV,
		distToLight).r;

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

	float3 lightResult;
	for (int i = 0; i < 5; i++) {
		switch (lights[i].Type) {
			case 0:
				//float3 CalculateDirectionalLight(Light incomingLight, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness, float metalness, float3 specularColor) {
				lightResult += CalculateDirectionalLight(lights[i], input.normal, colorTint, cameraPosition, input.worldPosition, roughness, metalness, specularColor);

				// If this is the first light, apply the shadowing result
				if (i == 0) {
					lightResult *= shadowAmount;
				}

				break;

			case 1:
				lightResult += CalculatePointLight(lights[i], input.normal, colorTint, ambient, cameraPosition, input.worldPosition, roughness, specularColor);
				break;

			case 2:
				lightResult += float3(0, 0, 0);
				break;

			default:
				lightResult += float3(0, 0, 0);
				break;
		}

		// Add this light's result to the total light for this pixel
		finalLight += lightResult;
	}

	finalLight *= albedo;

	return float4(pow(finalLight, 1.0f / 2.2f), 1);
}