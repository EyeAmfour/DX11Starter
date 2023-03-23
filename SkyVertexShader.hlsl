#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0) {
	//matrix world;
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
	//Order matters, the matrix goes BEFORE the float 4 in the multiplication
	matrix vp = mul(projection, viewNoTranslation);
	output.position = mul(vp, float4(input.localPosition, 1.0f));

	//Ensure output depth of each vertex will be exactly 1.0 after the shader
	output.position = output.position.xyww;

	//Set sample direction for vertex from center of the object
	output.sampleDir = input.localPosition;

	return output;
}