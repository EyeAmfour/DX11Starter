#include "Material.h"

Material::Material(DirectX::XMFLOAT4 tint, float roughness, std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps) :
colorTint(tint),
roughness(roughness),
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

float Material::GetRoughness() {
    return roughness;
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

void Material::SetRoughness(float roughness) {
    Material::roughness = roughness;
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

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv) {
    textureSRVs.insert({ name, srv });
}

void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState) {
    samplerOptions.insert({ name, samplerState });
}

void Material::PrepareMaterial() {
    for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
    for (auto& s : samplerOptions) { pixelShader->SetSamplerState(s.first.c_str(), s.second); }
}
