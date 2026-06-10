#pragma once
#include "Component.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Texture2D.h"
#include <string>
#include <memory>

namespace dae
{
    class SpritesheetComponent final : public Component
    {
    public:
        SpritesheetComponent(GameObject* pOwner, const std::string& filename, int frameWidth, int frameHeight, float scale = 1.f)
            : Component(pOwner)
            , m_frameWidth(frameWidth)
            , m_frameHeight(frameHeight)
            , m_scale(scale)
            , m_col(0)
            , m_row(0)
        {
            m_texture = ResourceManager::GetInstance().LoadTexture(filename);
        }

        ~SpritesheetComponent() override = default;

        SpritesheetComponent(const SpritesheetComponent&) = delete;
        SpritesheetComponent(SpritesheetComponent&&) = delete;
        SpritesheetComponent& operator=(const SpritesheetComponent&) = delete;
        SpritesheetComponent& operator=(SpritesheetComponent&&) = delete;

        void Update(float) override {}

        void Render() const override
        {
            if (!m_texture) return;
            const auto& pos = GetOwner()->GetWorldPosition();
            if (m_scale == 1.f)
            {
                Renderer::GetInstance().RenderTextureSrc(
                    *m_texture,
                    pos.x, pos.y,
                    m_col * m_frameWidth,
                    m_row * m_frameHeight,
                    m_frameWidth,
                    m_frameHeight
                );
            }
            else
            {
                Renderer::GetInstance().RenderTextureSrcScaled(
                    *m_texture,
                    pos.x, pos.y,
                    m_col * m_frameWidth,
                    m_row * m_frameHeight,
                    m_frameWidth,
                    m_frameHeight,
                    m_scale
                );
            }
        }

        void SetFrame(int col, int row)
        {
            m_col = col;
            m_row = row;
        }

        int GetFrameWidth() const { return m_frameWidth; }
        int GetFrameHeight() const { return m_frameHeight; }
        float GetScale() const { return m_scale; }

    private:
        std::shared_ptr<Texture2D> m_texture;
        int m_frameWidth;
        int m_frameHeight;
        float m_scale;
        int m_col;
        int m_row;
    };
}