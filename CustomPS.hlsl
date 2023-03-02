#include "ShaderIncludes.hlsli"

//Define Constant Buffer
cbuffer ExternalData : register(b0) {
	float4 colorTint;
	float time;
}

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
	//Modified from https://thebookofshaders.com/11/
	float xFloor = floor(input.screenPosition.x);
	float yFloor = floor(input.screenPosition.y);
	float xFrac = frac(input.screenPosition.x);
	float yFrac = frac(input.screenPosition.y);

	float2 pos = float2(xFloor, yFloor) * time;
	float2 fraction = float2(xFrac, yFrac);

	float a = random(pos);
	float b = random(pos + float2(1, 0));
	float c = random(pos + float2(0, 1));
	float d = random(pos + float2(1, 1));

	float2 u = mul(fraction, mul(fraction, 3.0 - mul(2.0, fraction)));

	float mixed = lerp(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;

	//Fade through colors by manipulating sine functions
	float red = mixed * colorTint.r * ((0.5 * sin(time + 5)) + 0.5);
	float green = mixed * colorTint.g * ((0.5 * sin(time + 7)) + 0.5);
	float blue = mixed * colorTint.b * ((0.5 * sin(time -3 )) + 0.5);

	return float4(red, green, blue, 1);
}