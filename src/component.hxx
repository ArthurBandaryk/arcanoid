#pragma once

#include <engine.hxx>

namespace arcanoid
{
    struct position
    {
        // Top left edge of the entity.
        float x {};
        float y {};
    };

    struct bound
    {
        float width {};
        float height {};
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
}
