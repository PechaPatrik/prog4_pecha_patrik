#include <algorithm>
#include "Scene.h"

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	m_objects.emplace_back(std::move(object));
}

void Scene::Remove(const GameObject& object)
{
	for (auto& go : m_objects) 
	{
		if (go.get() == &object) 
		{
			go->MarkForRemoval();
		}
	}
}

void Scene::RemoveAll()
{
	m_objects.clear();
}

void Scene::Update(float deltaTime)
{
	for(auto& object : m_objects)
	{
		if (!object->IsMarkedForRemoval()) 
		{
			object->Update(deltaTime);
		}
	}

	std::erase_if(m_objects, [](const auto& object) {
		return object->IsMarkedForRemoval();
		});
}

void dae::Scene::FixedUpdate(float fixedTimeStep)
{
	for (auto& object : m_objects)
	{
		if (!object->IsMarkedForRemoval())
		{
			object->FixedUpdate(fixedTimeStep);
		}
	}
}

void Scene::Render() const
{
	for (const auto& object : m_objects)
	{
		object->Render();
	}
}

