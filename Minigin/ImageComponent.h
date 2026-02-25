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
        ImageComponent(GameObject* pOwner, const std::string& filename);
        virtual ~ImageComponent() = default;

        ImageComponent(const ImageComponent& other) = delete;
        ImageComponent(ImageComponent&& other) = delete;
        ImageComponent& operator=(const ImageComponent& other) = delete;
        ImageComponent& operator=(ImageComponent&& other) = delete;

        void Update(float) override {}
        void Render() const override;

    private:
        std::shared_ptr<Texture2D> m_texture{};
    };
}