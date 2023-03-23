#ifndef __GGP_SHADER_INCLUDES__ // Each .hlsli file needs a unique identifier!
#define __GGP_SHADER_INCLUDES__
// ALL of your code pieces (structs, functions, etc.) go here!

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f

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

//Calculate Directional Lighting
float3 CalculateDirectionalLight(Light incomingLight, float3 normal, float4 surfaceColor, float3 ambient, float3 cameraPos, float3 worldPos, float roughness, float specTex) {
	float3 incomingLightDirection = normalize(incomingLight.Direction);

	Light light = incomingLight;
	light.Direction = normalize(-incomingLightDirection);

	float3 diffuse = Diffuse(normal, light.Direction);
	float3 finalColor = (diffuse * light.Color * surfaceColor.xyz) + (ambient * surfaceColor.xyz);

	float3 V = normalize(cameraPos - worldPos);
	float3 R = reflect(incomingLightDirection, normal);
	float spec = Specular(R, V, roughness) * specTex;
	spec *= any(diffuse);

	return surfaceColor.xyz * (finalColor + spec);
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