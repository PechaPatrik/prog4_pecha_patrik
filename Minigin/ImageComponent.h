#pragma once
#include <string>
#include <memory>
#include "Component.h"

namespace dae
{
    class Texture2D;

    class ImageComponent final : public Component
    {
    public:
        ImageComponent(GameObject* pOwner, const std::string& filename, float scale = 1.f);
        virtual ~ImageComponent() = default;

        ImageComponent(const ImageComponent& other) = delete;
        ImageComponent(ImageComponent&& other) = delete;
        ImageComponent& operator=(const ImageComponent& other) = delete;
        ImageComponent& operator=(ImageComponent&& other) = delete;

        void Update(float) override {}
        void Render() const override;

        void SetVisible(bool visible) { m_visible = visible; }
        bool IsVisible() const { return m_visible; }

    private:
        std::shared_ptr<Texture2D> m_texture{};
        float m_scale{ 1.f };
        bool m_visible{ true };
    };
}