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
                && a_coordinator.positions.count(i))
            {
                const position pos = a_coordinator.positions.at(i);
                arci::itexture* texture = a_coordinator.sprites.at(i).texture;
                arci::CHECK_NOTNULL(texture);

                std::vector<arci::vertex> vertices {};

                auto from_world_to_ndc = [this](const glm::vec2& world_pos) {
                    return glm::vec2 { -1.f + world_pos[0] * 2.f / screen_width,
                                       1.f - world_pos[1] * 2 / screen_height };
                };

                glm::vec2 tex_coordinates[4] {
                    glm::vec2 { 0.f, 1.f }, // Top left.
                    glm::vec2 { 1.f, 1.f }, // Top right.
                    glm::vec2 { 1.f, 0.f }, // Bottom right.
                    glm::vec2 { 0.f, 0.f }, // Bottom left.
                };

                for (std::size_t i = 0; i < pos.vertices.size(); i++)
                {
                    glm::vec2 ndc_pos = from_world_to_ndc(pos.vertices[i]);
                    arci::vertex v {
                        ndc_pos[0],
                        ndc_pos[1],
                        1.f,
                        0.f,
                        0.f,
                        0.f,
                        1.f,
                        tex_coordinates[i].x,
                        tex_coordinates[i].y
                    };

                    vertices.push_back(v);
                }

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
                position& pos = a_coordinator.positions.at(i);
                for (auto& vertex : pos.vertices)
                {
                    vertex.x
                        += a_coordinator.transformations.at(i).speed_x * dt;
                    vertex.y
                        += a_coordinator.transformations.at(i).speed_y * dt;
                }
            }
        }
    }

    void input_system::update(coordinator& a_coordinator,
                              arci::iengine* engine,
                              [[maybe_unused]] const float dt)
    {
        const float t = 1.f / 60.f;
        const float speed { 15.f / t };

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

    bool collision_system::are_collidable(const position& pos1,
                                          const position& pos2)
    {
        const float x1_left { pos1.vertices[0].x };
        const float x1_right { pos1.vertices[1].x };
        const float y1_top { pos1.vertices[0].y };
        const float y1_bottom { pos1.vertices[2].y };

        const float x2_left { pos2.vertices[0].x };
        const float x2_right { pos2.vertices[1].x };
        const float y2_top { pos2.vertices[0].y };
        const float y2_bottom { pos2.vertices[2].y };

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
        position& pos = a_coordinator.positions.at(id);

        for (const auto& vertex : pos.vertices)
        {
            const float new_x
                = vertex.x + a_coordinator.transformations.at(id).speed_x * dt;
            if (new_x < 0.f || new_x > screen_width)
            {
                a_coordinator.transformations.at(id).speed_x = 0.f;
                break;
            }
        }
    }

    void collision_system::resolve_ball_vs_walls(
        const entity id,
        coordinator& a_coordinator,
        const float dt,
        const std::size_t screen_width)
    {
        const position& pos = a_coordinator.positions.at(id);

        for (const auto& vertex : pos.vertices)
        {
            const float new_x = vertex.x
                + a_coordinator.transformations.at(id).speed_x * dt;
            const float new_y = vertex.y
                + a_coordinator.transformations.at(id).speed_y * dt;
            if (new_x < 0.f || new_x > screen_width)
            {
                a_coordinator.sounds["hit_ball"]->play(
                    arci::iaudio_buffer::running_mode::once);
                a_coordinator.transformations.at(id).speed_x *= -1.f;
            }

            if (new_y < 0.f)
            {
                a_coordinator.sounds["hit_ball"]->play(
                    arci::iaudio_buffer::running_mode::once);
                a_coordinator.transformations.at(id).speed_y *= -1.f;
                break;
            }
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
            // Entity is not collidable. So just continue.
            if (!a_coordinator.collidable_entities.count(ent))
            {
                continue;
            }

            // There is no any need to check collision to itself.
            if (ent == id)
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
        position& ball_pos = a_coordinator.positions.at(ball_id);
        position& brick_pos = a_coordinator.positions.at(brick_id);

        if (!are_collidable(brick_pos, ball_pos))
        {
            return;
        }
        else
        {
            // Ball collides with brick. Just remove the brick
            // from all data.
            a_coordinator.destroy_entity(brick_id);
        }

        // Reflect the ball if we've not done this on this frame.
        if (!is_collidable)
        {
            is_collidable = true;

            a_coordinator.sounds["hit_ball"]->play(
                arci::iaudio_buffer::running_mode::once);

            reflect_ball_from_brick(ball_id,
                                    ball_pos,
                                    brick_pos,
                                    a_coordinator);
        }
    }

    void collision_system::reflect_ball_from_brick(
        const entity ball_id,
        const position& ball_pos,
        const position& brick_pos,
        coordinator& a_coordinator)
    {
        const float ball_x_left { ball_pos.vertices[0].x };
        const float ball_x_right { ball_pos.vertices[1].x };
        const float ball_y_top { ball_pos.vertices[0].y };
        const float ball_y_bottom { ball_pos.vertices[2].y };

        const float brick_x_left { brick_pos.vertices[0].x };
        const float brick_x_right { brick_pos.vertices[1].x };
        const float brick_y_top { brick_pos.vertices[0].y };
        const float brick_y_bottom { brick_pos.vertices[2].y };

        const float ball_w_half { (ball_x_right - ball_x_left) / 2.f };
        const float ball_h_half { (ball_y_bottom - ball_y_top) / 2.f };
        const float cx = ball_x_left + ball_w_half;
        const float cy = ball_y_top + ball_h_half;

        // First case. Ball intersects only horizontal line of brick.
        if (cx <= brick_x_right && cx >= brick_x_left)
        {
            a_coordinator.transformations.at(ball_id).speed_y *= -1.f;
            return;
        }

        // Second case. Ball intersects only vertical line of brick.
        if (cy <= brick_y_bottom && cy >= brick_y_top)
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
        position& ball_pos = a_coordinator.positions.at(ball_id);
        position& platform_pos = a_coordinator.positions.at(platform_id);

        if (!are_collidable(platform_pos, ball_pos))
        {
            return;
        }
        else
        {
            a_coordinator.sounds["hit_ball"]->play(
                arci::iaudio_buffer::running_mode::once);

            reflect_ball_from_platform(ball_id,
                                       ball_pos,
                                       platform_pos,
                                       a_coordinator,
                                       dt);
        }
    }

    void collision_system::reflect_ball_from_platform(
        const entity ball_id,
        const position& ball_pos,
        const position& platform_pos,
        coordinator& a_coordinator,
        float dt)
    {
        const float ball_x_left { ball_pos.vertices[0].x };
        const float ball_x_right { ball_pos.vertices[1].x };
        const float ball_y_top { ball_pos.vertices[0].y };
        const float ball_y_bottom { ball_pos.vertices[2].y };

        const float platform_x_left { platform_pos.vertices[0].x };
        const float platform_x_right { platform_pos.vertices[1].x };
        const float platform_y_top { platform_pos.vertices[0].y };
        const float platform_y_bottom { platform_pos.vertices[2].y };

        const float ball_w_half { (ball_x_right - ball_x_left) / 2.f };
        const float ball_h_half { (ball_y_bottom - ball_y_top) / 2.f };
        const float cx = ball_x_left + ball_w_half;
        const float cy = ball_y_top + ball_h_half;

        // Set reflection angle for X axis.
        const float platform_w_half
            = (platform_x_right - platform_x_left) / 2.f;
        const float platform_center_x = platform_w_half + platform_x_left;
        const float delta = std::abs(cx - platform_center_x);
        const float tau = delta / platform_w_half;
        const float v1 = 0.f;
        float v2 = 7.f / dt;

        transform2d& tr = a_coordinator.transformations.at(ball_id);

        if (cx < platform_center_x)
        {
            v2 = -v2;
            tr.speed_x = v1 + (v2 - v1) * tau;
        }
        else
        {
            tr.speed_x = v1 + (v2 - v1) * tau;
        }

        // First case. Ball intersects only horizontal line of platform.
        if (cx <= platform_x_right && cx >= platform_x_left)
        {
            tr.speed_y *= -1.f;
            return;
        }

        // Second case. Ball intersects only vertical line of platform.
        if (cy <= platform_y_bottom && cy >= platform_y_top)
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
        const position& ball_pos = a_coordinator.positions.at(ball_id);

        if (ball_pos.vertices[2].y > screen_height)
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
