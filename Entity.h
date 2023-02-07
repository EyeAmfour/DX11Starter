#pragma once

#include <memory>

#include "Transform.h"
#include "Mesh.h"

class Entity {
private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;

public:
	//Constructor & Destructor
	Entity(std::shared_ptr<Mesh> mesh);
	~Entity();

	//Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

	void Draw();
};

