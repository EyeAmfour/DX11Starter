#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0) {
	matrix view;
	matrix projection;
}

VertexToPixel_Sky main(VertexShaderInput input)
{
	//Variable to be returned at end of main
	VertexToPixel_Sky output;

	//Copy view matrix and set translations to all 0
	matrix viewNoTranslation = view;

	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	//Apply projection and updated view to input position
	output.position = mul(float4(input.localPosition, 1), mul(projection, viewNoTranslation)).xyww;

	//Ensure output depth of each vertex will be exactly 1.0 after the shader
	//output.position = output.position.xyww;

	//Set sample direction for vertex from center of the object
	//output.sampleDir = float3(input.localPosition.xy, -input.localPosition.z);
	output.sampleDir = input.localPosition;

	return output;
}