#include "Entity.h"

#include <iostream>

Entity::Entity(std::shared_ptr<Mesh> mesh) : mesh(mesh) {
    transform = std::make_shared<Transform>();
}

Entity::~Entity() {
}

std::shared_ptr<Mesh> Entity::GetMesh() {
    return mesh;
}

std::shared_ptr<Transform> Entity::GetTransform() {
    return transform;
}

void Entity::Draw() {
    mesh.get()->Draw();
}
