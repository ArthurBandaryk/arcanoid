#pragma once

#include <engine.hxx>

#include <glm/ext/vector_float2.hpp>

#include <array>

namespace arcanoid
{
    struct position
    {
        std::array<glm::vec2, 4u> vertices;
    };

    struct sprite
    {
        arci::itexture* texture { nullptr };
    };

    struct transform2d
    {
        float speed_x { 0.f };
        float speed_y { 0.f };
    };

    struct key_inputs
    {
    };

    struct collision
    {
    };

    struct life
    {
        std::uint32_t lives_number {};
        std::uint32_t lives_left {};
    };

}
