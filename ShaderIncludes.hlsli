#ifndef __GGP_SHADER_INCLUDES__ // Each .hlsli file needs a unique identifier!
#define __GGP_SHADER_INCLUDES__
// ALL of your code pieces (structs, functions, etc.) go here!

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f

// The fresnel value for non-metals (dielectrics)
// Page 9: "F0 of nonmetals is now a constant 0.04"
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
static const float F0_NON_METAL = 0.04f;
// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal
// Handy to have this as a constant
static const float PI = 3.14159265359f;

// Struct representing a single vertex worth of data
struct VertexShaderInput {
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;
	float2 uv				: TEXCOORD;
	float3 tangent			: TANGENT;
};

// Struct representing the data we're sending down the pipeline
struct VertexToPixel {
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 worldPosition	: POSITION;
	float3 tangent			: TANGENT;
};

struct VertexToPixel_Sky {
	float4 position			: SV_POSITION;
	float3 sampleDir		: DIRECTION;
};

//Light struct
struct Light {
	int Type : L_TYPE;
	float3 Direction	: L_DIRECTION;
	float Range : L_RANGE;
	float3 Position		: L_POSITION;
	float Intensity : L_INTENSITY;
	float3 Color		: L_COLOR;
	float SpotFalloff : L_FALLOFF;
	float3 Padding		: L_PADDING;
};

//Pseudo-Random Function for noise
//Implemented from assignment 6 notes
float random(float2 s) {
	return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}

//N dot L Diffuse Lighting Equation
float3 Diffuse(float3 normal, float3 dirToLight) {
	return saturate(dot(normal, dirToLight));
}

//Phong Specular Calculation
float Specular(float3 R, float3 V, float roughness) {
	if (roughness > 0.05f) {
		float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
		return pow(saturate(dot(R, V)), specExponent);
	} else {
		return 0.0f;
	}
}

//Attenuation Calculation
float Attenuate(Light light, float3 worldPos) {
	float dist = distance(light.Position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

// Calculates diffuse amount based on energy conservation
//
// diffuse - Diffuse amount
// F - Fresnel result from microfacet BRDF
// metalness - surface metalness amount
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness) {
	return diffuse * (1 - F) * (1 - metalness);
}

// Normal Distribution Function: GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector: (V + L)/2
// n - Normal
//
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 n, float3 h, float roughness) {
	// Pre-calculations
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!
	// ((n dot h)^2 * (a^2 - 1) + 1)
	// Can go to zero if roughness is 0 and NdotH is 1; MIN_ROUGHNESS helps here
	float denomToSquare = NdotH2 * (a2 - 1) + 1;
	// Final value
	return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
//
// v - View vector
// h - Half vector
// f0 - Value when l = n
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 v, float3 h, float3 f0) {
	// Pre-calculations
	float VdotH = saturate(dot(v, h));
	// Final value
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric Shadowing - Schlick-GGX
// - k is remapped to a / 2, roughness remapped to (r+1)/2 before squaring!
//
// n - Normal
// v - View vector
//
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
//
// Full G(n,v,l,a) term = G_SchlickGGX(n,v,a) * G_SchlickGGX(n,l,a)
float G_SchlickGGX(float3 n, float3 v, float roughness) {
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));
	// Final value
	// Note: Numerator should be NdotV (or NdotL depending on parameters).
	// However, these are also in the BRDF's denominator, so they'll cancel!
	// We're leaving them out here AND in the BRDF function as the
	// dot products can get VERY small and cause rounding errors.
	return 1 / (NdotV * (1 - k) + k);
}

// Cook-Torrance Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - parts of the denominator are canceled out by numerator (see below)
//
// D() - Normal Distribution Function - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 specColor, out float3 F_out) {
	// Other vectors
	float3 h = normalize(v + l); // That’s an L, not a 1! Careful copy/pasting from a PDF!
	// Run numerator functions
	float D = D_GGX(n, h, roughness);
	float3 F = F_Schlick(v, h, F0_NON_METAL);
	float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
	// Pass F out of the function for diffuse balance
	F_out = F;
	// Final specular formula
	// Note: The denominator SHOULD contain (NdotV)(NdotL), but they'd be
	// canceled out by our G() term. As such, they have been removed
	// from BOTH places to prevent floating point rounding errors.
	float3 specularResult = (D * F * G) / 4;
	// One last non-obvious requirement: According to the rendering equation,
	// specular must have the same NdotL applied as diffuse! We'll apply
	// that here so that minimal changes are required elsewhere.
	return specularResult * max(dot(n, l), 0);
}

//Calculate Directional Lighting
float3 CalculateDirectionalLight(Light incomingLight, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness, float metalness, float3 specularColor) {
	float3 incomingLightDirection = normalize(incomingLight.Direction);

	Light light = incomingLight;
	light.Direction = normalize(-incomingLightDirection);

	//Lambert diffuse BRDF
	float3 diffuse = Diffuse(normal, light.Direction);
	//float3 finalColor = (diffuse * light.Color * surfaceColor.xyz); /* + (ambient * surfaceColor.xyz);*/

	float3 V = normalize(cameraPos - worldPos);
	float3 F;

	//PBR specular BRDF
	float3 spec = MicrofacetBRDF(normal, light.Direction, V, roughness, specularColor, F);

	// Calculate diffuse with energy conservation, including cutting diffuse for metals
	float3 balancedDiff = DiffuseEnergyConserve(diffuse, F, metalness);

	// Combine the final diffuse and specular values for this light
	float3 total = (balancedDiff * surfaceColor + spec) * light.Intensity * light.Color;

	/*float3 R = reflect(incomingLightDirection, normal);
	float spec = Specular(R, V, roughness) * specTex;
	spec *= any(diffuse);*/

	return total;
}

//Calculate Point Lighting
float3 CalculatePointLight(Light incomingLight, float3 normal, float4 surfaceColor, float3 ambient, float3 cameraPos, float3 worldPos, float roughness, float specTex) {
	float3 surfaceToLight = normalize(incomingLight.Position - worldPos);

	Light light = incomingLight;
	light.Direction = normalize(-surfaceToLight);


	float3 diffuse = Diffuse(normal, surfaceToLight);
	float3 finalColor = (diffuse * light.Color * surfaceColor.xyz) + (ambient * surfaceColor.xyz);

	float3 V = normalize(cameraPos - worldPos);
	float3 R = reflect(light.Direction, normal);
	float spec = Specular(R, V, roughness) * specTex;
	spec *= any(diffuse);

	float3 l = surfaceColor.xyz * (finalColor + spec);
	float attenuation = Attenuate(light, worldPos);

	return l * attenuation;
}
#endif