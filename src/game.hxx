#pragma once

#include "component.hxx"
#include "coordinator.hxx"
#include "engine.hxx"
#include "entity.hxx"
#include "game-system.hxx"

#include "FrameTimer.hxx"

#include <memory>

namespace arcanoid
{
    class game final
    {
    public:
        void main_loop();
        ~game();

    private:
        void on_init();
        void on_event();
        void on_update(float dt);
        void on_render();

        void init_world();
        void init_bricks();
        void init_ball();
        void init_platform();
        void init_background();

        std::vector<arci::itexture*> m_textures {};

        cFrameTimer m_frame_timer;
        coordinator m_coordinator {};
        input_system m_input_system {};
        sprite_system m_sprite_system {};
        transform_system m_transform_system {};
        collision_system m_collision_system {};
        game_over_system m_game_over_system {};
        menu_system m_menu_system {};

        std::unique_ptr<arci::iengine,
                        void (*)(arci::iengine*)>
            m_engine { nullptr, nullptr };
        std::size_t m_screen_w {};
        std::size_t m_screen_h {};
        game_status m_status { game_status::main_menu };
    };
}
