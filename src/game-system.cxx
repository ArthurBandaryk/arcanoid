#include "game-system.hxx"
#include "entity.hxx"
#include "helper.hxx"

#include <imgui.h>

#include <cmath>

namespace arcanoid
{
    void sprite_system::render(arci::iengine* engine,
                               coordinator& a_coordinator)
    {
        for (entity i = 1; i <= entities_number; i++)
        {
            if (a_coordinator.sprites.count(i)
                && a_coordinator.positions.count(i)
                && a_coordinator.bounds.count(i))
            {
                const position& top_left = a_coordinator.positions.at(i);
                arci::itexture* texture = a_coordinator.sprites.at(i).texture;
                arci::CHECK_NOTNULL(texture);

                const auto [w, h] = a_coordinator.bounds.at(i);

                // Translate from world to ndc coordinates.
                auto from_world_to_ndc = [this](const glm::vec2& world_pos) {
                    return glm::vec2 { -1.f + world_pos[0] * 2.f / screen_width,
                                       1.f - world_pos[1] * 2 / screen_height };
                };

                glm::vec2 top_right_ndc {
                    from_world_to_ndc({ top_left.pos.x + w, top_left.pos.y })
                };
                glm::vec2 bottom_right_ndc {
                    from_world_to_ndc({ top_left.pos.x + w,
                                        top_left.pos.y + h })
                };
                glm::vec2 bottom_left_ndc {
                    from_world_to_ndc({ top_left.pos.x, top_left.pos.y + h })
                };

                glm::vec2 top_left_ndc = from_world_to_ndc(top_left.pos);

                std::vector<arci::vertex> vertices {
                    { top_left_ndc.x, top_left_ndc.y, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f },
                    { top_right_ndc.x, top_right_ndc.y, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f },
                    { bottom_right_ndc.x, bottom_right_ndc.y, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
                    { bottom_left_ndc.x, bottom_left_ndc.y, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f },
                };

                std::vector<std::uint32_t> indices { 0, 1, 2, 0, 3, 2 };

                arci::ivertex_buffer* vbo = engine->create_vertex_buffer(vertices);
                arci::i_index_buffer* ebo = engine->create_ebo(indices);
                arci::CHECK_NOTNULL(vbo);
                arci::CHECK_NOTNULL(ebo);

                engine->render(vbo, ebo, texture);
                engine->destroy_vertex_buffer(vbo);
                engine->destroy_ebo(ebo);
            }
        }
    }

    void transform_system::update(coordinator& a_coordinator, const float dt)
    {
        for (entity i = 1; i <= entities_number; i++)
        {
            if (a_coordinator.transformations.count(i)
                && a_coordinator.positions.count(i))
            {
                position& top_left = a_coordinator.positions.at(i);
                const transform2d& tr = a_coordinator.transformations.at(i);

                top_left.pos.x += tr.speed_x * dt;
                top_left.pos.y += tr.speed_y * dt;
            }
        }
    }

    void input_system::update(coordinator& a_coordinator,
                              arci::iengine* engine,
                              [[maybe_unused]] const float dt)
    {
        const float speed { 15.f * 60.f };

        for (entity i = 1; i <= entities_number; i++)
        {
            if (a_coordinator.inputs.count(i)
                && a_coordinator.transformations.count(i))
            {
                if (engine->key_down(arci::keys::left))
                {
                    a_coordinator.transformations.at(i).speed_x = -speed;
                }
                if (engine->key_down(arci::keys::right))
                {
                    a_coordinator.transformations.at(i).speed_x = speed;
                }
                if (!engine->key_down(arci::keys::right)
                    && !engine->key_down(arci::keys::left))
                {
                    a_coordinator.transformations.at(i).speed_x = 0.f;
                }
            }
        }
    }

    void input_system::update(coordinator& a_coordinator)
    {
        const float speed { 15.f * 60.f };

        for (entity i = 1; i <= entities_number; i++)
        {
            if (!a_coordinator.inputs.count(i)
                || !a_coordinator.transformations.count(i))
            {
                continue;
            }

            transform2d& tr = a_coordinator.transformations.at(i);

            if (!event)
            {
                tr.speed_x = 0.f;
                continue;
            }

            const arci::event& e = event.value();

            if (!e.key_info || e.device == arci::event_from_device::none)
            {
                tr.speed_x = 0.f;
                continue;
            }

            if (*e.key_info == arci::key_event::left_button_pressed)
            {
                tr.speed_x = -speed;
            }

            if (*e.key_info == arci::key_event::right_button_pressed)
            {
                tr.speed_x = speed;
            }
        }
    }

    void collision_system::update(coordinator& a_coordinator,
                                  const float dt,
                                  const std::size_t screen_width)
    {
        for (auto& collidable : a_coordinator.collidable_entities)
        {
            if (collidable.first == a_coordinator.collidable_ids.at("ball"))
            {
                resolve_collision_for_ball(collidable.first,
                                           a_coordinator,
                                           dt,
                                           screen_width);
                continue;
            }

            if (collidable.first == a_coordinator.collidable_ids.at("platform"))
            {
                resolve_collision_for_platform(collidable.first,
                                               a_coordinator,
                                               dt,
                                               screen_width);
            }
        }
    }

    bool collision_system::are_collidable(const entity& ent1,
                                          const entity& ent2,
                                          const coordinator& a_coordinator)
    {
        const position& top_left1 = a_coordinator.positions.at(ent1);
        const position& top_left2 = a_coordinator.positions.at(ent2);

        const auto [w1, h1] = a_coordinator.bounds.at(ent1);
        const auto [w2, h2] = a_coordinator.bounds.at(ent2);

        const float x1_left { top_left1.pos.x };
        const float x1_right { top_left1.pos.x + w1 };
        const float y1_top { top_left1.pos.y };
        const float y1_bottom { top_left1.pos.y + h1 };

        const float x2_left { top_left2.pos.x };
        const float x2_right { top_left2.pos.x + w2 };
        const float y2_top { top_left2.pos.y };
        const float y2_bottom { top_left2.pos.y + h2 };

        // Check if collision occurs for X axis.
        if ((x2_left <= x1_right && x2_left >= x1_left)
            || (x2_right <= x1_right && x2_right >= x1_left))
        {
            // Check if collision occurs for Y axis.
            if ((y2_top <= y1_bottom && y2_top >= y1_top)
                || (y2_bottom <= y1_bottom && y2_bottom >= y1_top))
            {
                return true;
            }
        }

        return false;
    }

    void collision_system::resolve_collision_for_platform(
        const entity id,
        coordinator& a_coordinator,
        const float dt,
        const std::size_t screen_width)
    {
        position& top_left = a_coordinator.positions.at(id);
        transform2d& tr = a_coordinator.transformations.at(id);
        const auto [w, _] = a_coordinator.bounds.at(id);

        const float new_left_x = top_left.pos.x + tr.speed_x * dt;
        const float new_right_x = top_left.pos.x + w + tr.speed_x * dt;

        if (new_left_x <= 0.f)
        {
            top_left.pos.x = 0.f;
            tr.speed_x = 0.f;
        }

        if (new_right_x >= screen_width)
        {
            top_left.pos.x = screen_width - w;
            tr.speed_x = 0.f;
        }
    }

    void collision_system::resolve_ball_vs_walls(
        const entity id,
        coordinator& a_coordinator,
        const float dt,
        const std::size_t screen_width)
    {
        const position& top_left = a_coordinator.positions.at(id);
        transform2d& tr = a_coordinator.transformations.at(id);
        const auto [w, h] = a_coordinator.bounds.at(id);

        const float new_left_x = top_left.pos.x + tr.speed_x * dt;
        const float new_top_y = top_left.pos.y + tr.speed_y * dt;
        const float new_right_x = top_left.pos.x + w + tr.speed_x * dt;

        if (new_left_x < 0.f || new_right_x > screen_width)
        {
            a_coordinator.sounds["hit_ball"]->play(
                arci::iaudio_buffer::running_mode::once);
            tr.speed_x *= -1.f;
        }

        if (new_top_y < 0.f)
        {
            a_coordinator.sounds["hit_ball"]->play(
                arci::iaudio_buffer::running_mode::once);
            tr.speed_y *= -1.f;
        }
    }

    void collision_system::resolve_collision_for_ball(
        const entity id,
        coordinator& a_coordinator,
        const float dt,
        const std::size_t screen_width)
    {
        resolve_ball_vs_walls(id,
                              a_coordinator,
                              dt,
                              screen_width);

        // If the ball is collidable with some brick.
        // We're using this flag just to know that we should
        // not change the direction cause we've already
        // done it per this frame.
        bool is_collidable { false };

        for (entity ent = 1; ent <= entities_number; ent++)
        {
            // There is no any need to check collision to itself.
            if (ent == id)
            {
                continue;
            }

            // Entity is not collidable. So just continue.
            if (!a_coordinator.collidable_entities.count(ent))
            {
                continue;
            }

            if (ent == a_coordinator.collidable_ids.at("platform"))
            {
                resolve_ball_vs_platform(id, ent, a_coordinator, dt);
                return;
            }

            resolve_ball_vs_brick(id, ent, a_coordinator, is_collidable);
        }
    }

    void collision_system::resolve_ball_vs_brick(
        const entity ball_id,
        const entity brick_id,
        coordinator& a_coordinator,
        bool& is_collidable)
    {
        if (!are_collidable(brick_id, ball_id, a_coordinator))
        {
            return;
        }

        // Reflect the ball if we've not done this on this frame.
        if (!is_collidable)
        {
            is_collidable = true;

            a_coordinator.sounds["hit_ball"]->play(
                arci::iaudio_buffer::running_mode::once);

            reflect_ball_from_brick(ball_id, brick_id, a_coordinator);
        }

        // Ball collides with brick. Just remove the brick
        // from all data.
        a_coordinator.destroy_entity(brick_id);
    }

    void collision_system::reflect_ball_from_brick(
        const entity ball_id,
        const entity brick_id,
        coordinator& a_coordinator)
    {
        const position& top_left_ball = a_coordinator.positions.at(ball_id);
        const position& top_left_brick = a_coordinator.positions.at(brick_id);
        const auto [ball_w, ball_h] = a_coordinator.bounds.at(ball_id);
        const auto [brick_w, brick_h] = a_coordinator.bounds.at(brick_id);

        const float ball_x_left { top_left_ball.pos.x };
        const float ball_x_right { top_left_ball.pos.x + ball_w };
        const float ball_y_top { top_left_ball.pos.y };
        const float ball_y_bottom { top_left_ball.pos.y + ball_h };

        const float brick_x_left { top_left_brick.pos.x };
        const float brick_x_right { top_left_brick.pos.x + brick_w };
        const float brick_y_top { top_left_brick.pos.y };
        const float brick_y_bottom { top_left_brick.pos.y + brick_h };

        const float ball_w_half { (ball_x_right - ball_x_left) / 2.f };
        const float ball_h_half { (ball_y_bottom - ball_y_top) / 2.f };
        const float ball_center_x = ball_x_left + ball_w_half;
        const float ball_center_y = ball_y_top + ball_h_half;

        // First case. Ball intersects only horizontal line of brick.
        if (ball_center_x <= brick_x_right && ball_center_x >= brick_x_left)
        {
            a_coordinator.transformations.at(ball_id).speed_y *= -1.f;
            return;
        }

        // Second case. Ball intersects only vertical line of brick.
        if (ball_center_y <= brick_y_bottom && ball_center_y >= brick_y_top)
        {
            a_coordinator.transformations.at(ball_id).speed_x *= -1.f;
            return;
        }
        // Ball intersects edge of the brick.
        else
        {
            transform2d& tr = a_coordinator.transformations.at(ball_id);

            if (tr.speed_y < 0.f)
            {
                tr.speed_y *= -1.f;
            }
            else
            {
                tr.speed_x *= -1.f;
            }
        }
    }

    void collision_system::resolve_ball_vs_platform(
        const entity ball_id,
        const entity platform_id,
        coordinator& a_coordinator,
        const float dt)
    {
        if (!are_collidable(platform_id, ball_id, a_coordinator))
        {
            return;
        }
        else
        {
            a_coordinator.sounds["hit_ball"]->play(
                arci::iaudio_buffer::running_mode::once);

            reflect_ball_from_platform(ball_id,
                                       platform_id,
                                       a_coordinator,
                                       dt);
        }
    }

    void collision_system::reflect_ball_from_platform(
        const entity ball_id,
        const entity platform_id,
        coordinator& a_coordinator,
        float dt)
    {
        const position& top_left_ball = a_coordinator.positions.at(ball_id);
        const position& top_left_platform = a_coordinator.positions.at(platform_id);
        const auto [ball_w, ball_h] = a_coordinator.bounds.at(ball_id);
        const auto [platform_w, platform_h] = a_coordinator.bounds.at(platform_id);

        const float ball_x_left { top_left_ball.pos.x };
        const float ball_x_right { top_left_ball.pos.x + ball_w };
        const float ball_y_top { top_left_ball.pos.y };
        const float ball_y_bottom { top_left_ball.pos.y + ball_h };

        const float platform_x_left { top_left_platform.pos.x };
        const float platform_x_right { top_left_platform.pos.x + platform_w };
        const float platform_y_top { top_left_platform.pos.y };
        const float platform_y_bottom { top_left_platform.pos.y + platform_h };

        const float ball_w_half { (ball_x_right - ball_x_left) / 2.f };
        const float ball_h_half { (ball_y_bottom - ball_y_top) / 2.f };
        const float ball_center_x = ball_x_left + ball_w_half;
        const float ball_center_y = ball_y_top + ball_h_half;

        // Set reflection angle for X axis.
        const float platform_w_half
            = (platform_x_right - platform_x_left) / 2.f;
        const float platform_center_x = platform_w_half + platform_x_left;
        const float delta = std::abs(ball_center_x - platform_center_x);
        const float tau = delta / platform_w_half;
        const float v1 = 0.f;
        float v2 = 7.f / dt;

        transform2d& tr = a_coordinator.transformations.at(ball_id);

        if (ball_center_x < platform_center_x)
        {
            v2 = -v2;
            tr.speed_x = v1 + (v2 - v1) * tau;
        }
        else
        {
            tr.speed_x = v1 + (v2 - v1) * tau;
        }

        // First case. Ball intersects only horizontal line of platform.
        if (ball_center_x <= platform_x_right && ball_center_x >= platform_x_left)
        {
            tr.speed_y *= -1.f;
            return;
        }

        // Second case. Ball intersects only vertical line of platform.
        if (ball_center_y <= platform_y_bottom && ball_center_y >= platform_y_top)
        {
            tr.speed_y = std::abs(tr.speed_y);
            return;
        }
        // Ball intersects edge of the platform.
        else
        {
            tr.speed_y = std::abs(tr.speed_y);
        }
    }

    void game_over_system::update(
        coordinator& a_coordinator,
        game_status& status,
        const std::size_t screen_height)
    {
        const entity ball_id = a_coordinator.collidable_ids.at("ball");
        const position& ball_top_left = a_coordinator.positions.at(ball_id);
        const auto [_, ball_h] = a_coordinator.bounds.at(ball_id);

        if (ball_top_left.pos.y + ball_h > screen_height)
        {
            status = game_status::game_over;
        }
    }

    void game_over_system::render(arci::iengine* engine,
                                  const std::size_t width,
                                  const std::size_t height)
    {
        engine->imgui_new_frame();

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x,
                                       main_viewport->WorkPos.y),
                                ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoResize;

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowBorderSize = 0.0f;

        ImGui::Begin("Main menu", nullptr, window_flags);

        // Game over text label.
        const char title[] { "GAME OVER!" };
        const float title_offset_y { height / 2.f };
        const float title_text_width = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((width - title_text_width) * 0.5f);
        ImGui::SetCursorPosY(title_offset_y);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.f, 0.f, 0.f, 255.f));
        ImGui::Text(title);
        ImGui::PopStyleColor();

        ImGui::End();

        engine->imgui_render();
    }

    void menu_system::render(arci::iengine* engine,
                             game_status& status,
                             std::size_t width,
                             const std::size_t height)
    {
        engine->imgui_new_frame();

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x,
                                       main_viewport->WorkPos.y),
                                ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoResize;

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowBorderSize = 0.0f;

        ImGui::Begin("Main menu", nullptr, window_flags);

        // Game name text label.
        const char game_name[] { "ARCANOID" };
        const float game_name_offset_y { height / 10.f };
        const float game_text_width = ImGui::CalcTextSize(game_name).x;
        const float game_text_height = ImGui::CalcTextSize(game_name).y;
        ImGui::SetCursorPosX((width - game_text_width) * 0.5f);
        ImGui::SetCursorPosY(game_name_offset_y);
        ImGui::Text(game_name);

        // Start game button.
        const char start_button_name[] { "START" };
        const float start_button_width { game_text_width * 2.f };
        const float start_button_height { game_text_height * 2.f };
        ImGui::SetCursorPosX((width - start_button_width) * 0.5f);
        ImGui::SetCursorPosY(game_name_offset_y
                             + game_text_height
                             + game_name_offset_y / 2.f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.f, 128.f, 0.f, 255.f));
        if (ImGui::Button(start_button_name,
                          ImVec2(start_button_width, start_button_height)))
        {
            status = game_status::game;
        }
        ImGui::PopStyleColor();

        // Exit button.
        const char exit_button_name[] { "EXIT" };
        const float exit_button_width { start_button_width };
        const float exit_button_height { start_button_height };
        ImGui::SetCursorPosX((width - exit_button_width) * 0.5f);
        ImGui::SetCursorPosY(game_name_offset_y
                             + game_text_height
                             + game_name_offset_y / 2.f
                             + start_button_height
                             + game_name_offset_y / 2.f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.f, 0.f, 0.f, 255.f));
        if (ImGui::Button(exit_button_name,
                          ImVec2(exit_button_width, exit_button_height)))
        {
            status = game_status::exit;
        }
        ImGui::PopStyleColor();

        ImGui::End();

        engine->imgui_render();
    }
}
