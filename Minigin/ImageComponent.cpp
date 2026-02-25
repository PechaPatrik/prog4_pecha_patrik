#include "ImageComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "GameObject.h"

using namespace dae;

ImageComponent::ImageComponent(GameObject* pOwner, const std::string& filename)
    : Component(pOwner)
{
    m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void ImageComponent::Render() const
{
    if (m_texture)
    {
        const auto& pos = GetOwner()->GetPosition();
        Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
    }
}