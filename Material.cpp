#include "Material.h"

Material::Material(DirectX::XMFLOAT4 tint, std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps) :
colorTint(tint),
vertexShader(vs),
pixelShader(ps) {

}

Material::~Material() {
}

// --------------------------------------------------------
// Get the ColorTint
// --------------------------------------------------------
DirectX::XMFLOAT4 Material::GetColorTint() {
    return colorTint;
}

// --------------------------------------------------------
// Gets the smart pointer to the Vertex Shader
// --------------------------------------------------------
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() {
    return vertexShader;
}

// --------------------------------------------------------
// Gets the smart pointer to the Pixel Shader
// --------------------------------------------------------
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() {
    return pixelShader;
}

// --------------------------------------------------------
// Sets the Tint
// --------------------------------------------------------
void Material::SetColorTint(DirectX::XMFLOAT4 tint) {
    colorTint = DirectX::XMFLOAT4(tint);
}

// --------------------------------------------------------
// Sets the Vertex Shader
// --------------------------------------------------------
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vs) {
    vertexShader = vs;
}

// --------------------------------------------------------
// Sets the Pixel Shader
// --------------------------------------------------------
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> ps) {
    pixelShader = ps;
}
