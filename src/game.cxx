#include "game.hxx"
#include "helper.hxx"

#include <chrono>

namespace arcanoid
{
    void game::main_loop()
    {
        on_init();

        bool loop_continue { true };

        while (loop_continue)
        {
            m_frame_timer.update();

            on_event();

            if (m_status == game_status::exit)
            {
                loop_continue = false;
                break;
            }

            const float frame_delta = m_frame_timer.getFrameDeltaTime();
            on_update(frame_delta);

            on_render();
        }
    }

    void game::on_event()
    {
        arci::event event;
        while (m_engine->process_input(event))
        {
            if (event.is_quitting)
            {
                m_status = game_status::exit;
                break;
            }
        }
    }

    void game::on_update(float dt)
    {
        if (m_status != game_status::game)
        {
            return;
        }

        // When debugging dt is too big. So set it being 1/60.
        dt = std::min(dt, 1.0f / 60.0f);

        m_game_over_system.update(m_coordinator, m_status, m_screen_h);
        m_input_system.update(m_coordinator, m_engine.get(), dt);
        m_collision_system.update(m_coordinator, dt, m_screen_w);
        m_transform_system.update(m_coordinator, dt);
    }

    void game::on_render()
    {
        if (m_status == game_status::game_over)
        {
            m_game_over_system.render(m_engine.get(),
                                      m_screen_w,
                                      m_screen_h);
        }
        else if (m_status == game_status::main_menu)
        {
            m_menu_system.render(m_engine.get(),
                                 m_status,
                                 m_screen_w,
                                 m_screen_h);
        }
        else
        {
            m_sprite_system.render(m_engine.get(), m_coordinator);
        }

        m_engine->swap_buffers();
    }

    void game::on_init()
    {
        m_engine = std::unique_ptr<arci::iengine, void (*)(arci::iengine*)> {
            arci::engine_create(),
            arci::engine_destroy
        };

        m_engine->init();

        const auto [w, h] = m_engine->get_screen_resolution();
        m_screen_w = w;
        m_screen_h = h;
        m_sprite_system.screen_width = w;
        m_sprite_system.screen_height = h;

        arci::iaudio_buffer* background_sound
            = m_engine->create_audio_buffer("res/music.wav");
        arci::iaudio_buffer* hit_ball_sound
            = m_engine->create_audio_buffer("res/hit.wav");
        m_coordinator.sounds.insert({ "background", background_sound });
        m_coordinator.sounds.insert({ "hit_ball", hit_ball_sound });

        m_coordinator.sounds["background"]->play(
            arci::iaudio_buffer::running_mode::for_ever);

        init_world();
        m_frame_timer.restart();
    }

    game::~game()
    {
        for (auto texture : m_textures)
        {
            m_engine->destroy_texture(texture);
        }

        m_engine->uninit();

        for (auto [_, audio_buffer] : m_coordinator.sounds)
        {
            m_engine->destroy_audio_buffer(audio_buffer);
        }

        m_engine->imgui_uninit();
    }

    void game::init_world()
    {
        init_background();
        init_bricks();
        init_ball();
        init_platform();
    }

    void game::init_bricks()
    {
        arci::itexture* yellow_brick_texture
            = m_engine->create_texture("res/yellow_brick.png");
        arci::CHECK_NOTNULL(yellow_brick_texture);
        m_textures.push_back(yellow_brick_texture);

        const float brick_width { m_screen_w / 10.f };
        const float brick_height { m_screen_h / 20.f };

        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                entity brick = create_entity();

                position brick_position {
                    glm::vec2 { 0.f + brick_width * j,
                                0.f + brick_height * i },
                    glm::vec2 { brick_width + brick_width * j,
                                0.f + brick_height * i },
                    glm::vec2 { brick_width + brick_width * j,
                                brick_height + brick_height * i },
                    glm::vec2 { 0.f + brick_width * j,
                                brick_height + brick_height * i },
                };
                const auto [it1, position_inserted]
                    = m_coordinator.positions.insert({ brick, brick_position });
                arci::CHECK(position_inserted);

                sprite brick_sprite { yellow_brick_texture };
                const auto [it2, sprite_inserted]
                    = m_coordinator.sprites.insert({ brick, brick_sprite });
                arci::CHECK(sprite_inserted);

                collision collision_component {};
                const auto [it3, collision_inserted]
                    = m_coordinator.collidable_entities.insert(
                        { brick, collision_component });
                arci::CHECK(collision_inserted);
            }
        }
    }

    void game::init_background()
    {
        entity background = create_entity();

        arci::itexture* background_texture
            = m_engine->create_texture("res/background1.png");
        arci::CHECK_NOTNULL(background_texture);
        m_textures.push_back(background_texture);

        position pos {
            glm::vec2 { 0.f, 0.f },
            glm::vec2 { m_screen_w, 0.f },
            glm::vec2 { m_screen_w, m_screen_h },
            glm::vec2 { 0.f, m_screen_h },
        };
        const auto [it1, pos_inserted]
            = m_coordinator.positions.insert({ background, pos });
        arci::CHECK(pos_inserted);

        sprite spr { background_texture };
        const auto [it2, sprite_inserted]
            = m_coordinator.sprites.insert({ background, spr });
        arci::CHECK(sprite_inserted);
    }

    void game::init_ball()
    {
        entity ball = create_entity();

        arci::itexture* texture
            = m_engine->create_texture("res/ball.png");
        arci::CHECK_NOTNULL(texture);
        m_textures.push_back(texture);

        const float ball_width { m_screen_w / 45.f };
        const float ball_height { m_screen_w / 45.f };

        position pos {
            glm::vec2 { m_screen_w / 2.f - ball_width / 2.f,
                        3.f * m_screen_h / 4.f - ball_height / 2.f },
            glm::vec2 { m_screen_w / 2.f + ball_width / 2.f,
                        3.f * m_screen_h / 4.f - ball_height / 2.f },
            glm::vec2 { m_screen_w / 2.f + ball_width / 2.f,
                        3.f * m_screen_h / 4.f + ball_height / 2.f },
            glm::vec2 { m_screen_w / 2.f - ball_width / 2.f,
                        3.f * m_screen_h / 4.f + ball_height / 2.f },
        };
        const auto [it1, pos_inserted]
            = m_coordinator.positions.insert({ ball, pos });
        arci::CHECK(pos_inserted);

        sprite spr { texture };
        const auto [it2, sprite_inserted]
            = m_coordinator.sprites.insert({ ball, spr });
        arci::CHECK(sprite_inserted);

        transform2d transform { -60.f, -360.f };
        const auto [it3, transform_inserted]
            = m_coordinator.transformations.insert({ ball, transform });
        arci::CHECK(transform_inserted);

        collision collision_component {};
        const auto [it4, collision_inserted]
            = m_coordinator.collidable_entities.insert(
                { ball, collision_component });
        arci::CHECK(collision_inserted);

        const auto [it5, collision_id_inserted]
            = m_coordinator.collidable_ids.insert({ "ball", ball });
        arci::CHECK(collision_id_inserted);
    }

    void game::init_platform()
    {
        entity platform = create_entity();

        arci::itexture* texture
            = m_engine->create_texture("res/platform1.png");
        arci::CHECK_NOTNULL(texture);
        m_textures.push_back(texture);

        const float platform_width { m_screen_w / 6.f };
        const float platform_height { m_screen_w / 35.f };

        position pos {
            glm::vec2 { m_screen_w / 2.f - platform_width / 2.f,
                        m_screen_h - platform_height },
            glm::vec2 { m_screen_w / 2.f + platform_width / 2.f,
                        m_screen_h - platform_height },
            glm::vec2 { m_screen_w / 2.f + platform_width / 2.f,
                        m_screen_h },
            glm::vec2 { m_screen_w / 2.f - platform_width / 2.f,
                        m_screen_h },
        };
        const auto [it1, pos_inserted]
            = m_coordinator.positions.insert({ platform, pos });
        arci::CHECK(pos_inserted);

        sprite spr { texture };
        const auto [it2, sprite_inserted]
            = m_coordinator.sprites.insert({ platform, spr });
        arci::CHECK(sprite_inserted);

        transform2d transform {};
        const auto [it3, transform_inserted]
            = m_coordinator.transformations.insert({ platform, transform });
        arci::CHECK(transform_inserted);

        key_inputs input {};
        const auto [it4, input_inserted]
            = m_coordinator.inputs.insert({ platform, input });
        arci::CHECK(input_inserted);

        collision collision_component {};
        const auto [it5, collision_inserted]
            = m_coordinator.collidable_entities.insert(
                { platform, collision_component });
        arci::CHECK(collision_inserted);

        const auto [it6, collision_id_inserted]
            = m_coordinator.collidable_ids.insert({ "platform", platform });
        arci::CHECK(collision_id_inserted);
    }
}
