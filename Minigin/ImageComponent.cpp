#include "ImageComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "GameObject.h"

using namespace dae;

ImageComponent::ImageComponent(GameObject* pOwner, const std::string& filename, float scale)
    : Component(pOwner)
    , m_scale(scale)
{
    m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void ImageComponent::Render() const
{
    if (!m_visible || !m_texture) return;
    const auto& pos = GetOwner()->GetWorldPosition();
    if (m_scale == 1.f)
    {
        Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
    }
    else
    {
        glm::vec2 size = m_texture->GetSize();
        Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y, size.x * m_scale, size.y * m_scale);
    }
}