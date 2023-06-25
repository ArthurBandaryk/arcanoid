#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include <glm/ext/matrix_float2x2_precision.hpp>

///////////////////////////////////////////////////////////////////////////////

namespace arci
{

    ///////////////////////////////////////////////////////////////////////////////

    class iengine;

    iengine* engine_create();

    ///////////////////////////////////////////////////////////////////////////////

    void engine_destroy(iengine* engine);

    ///////////////////////////////////////////////////////////////////////////////

    enum class event_from_device
    {
        keyboard,
        mouse,
        touch,
        none
    };

    ///////////////////////////////////////////////////////////////////////////////

    struct mouse_event
    {
        float x;
        float y;
    };

    ///////////////////////////////////////////////////////////////////////////////

    enum class key_event
    {
        left_button_pressed,
        left_button_released,
        right_button_pressed,
        right_button_released,
        up_button_pressed,
        up_button_released,
        down_button_pressed,
        down_button_released,
        button1_pressed,
        button1_released,
        button2_pressed,
        button2_released,
        button3_pressed,
        button3_released,
        escape_button_pressed,
        escape_button_released,
    };

    enum class keys
    {
        left,
        right,
        down,
        up,
        select,
        magnify,
        reduce,
        button1,
        button2,
        exit
    };

    ///////////////////////////////////////////////////////////////////////////////

    struct event
    {
        event_from_device device { event_from_device::none };
        std::optional<mouse_event> mouse_info;
        std::optional<key_event> key_info;
        bool is_quitting { false };
    };

    ///////////////////////////////////////////////////////////////////////////////

    struct vertex
    {
        float x {};
        float y {};
        float z {};
        float r {};
        float g {};
        float b {};
        float a {};
        float tx {};
        float ty {};
    };

    struct triangle
    {
        triangle() = default;
        triangle(const vertex& v0, const vertex& v1, const vertex& v2);

        std::array<vertex, 3> vertices {};
    };

    ///////////////////////////////////////////////////////////////////////////////

    class ivertex_buffer
    {
    public:
        virtual ~ivertex_buffer() = default;
        virtual void bind() = 0;
        virtual std::size_t get_vertices_number() const = 0;
    };

    class i_index_buffer
    {
    public:
        virtual ~i_index_buffer() = default;
        virtual void bind() = 0;
        virtual uint32_t* data() = 0;
        virtual std::size_t get_indices_number() const = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////

    class itexture
    {
    public:
        virtual ~itexture() = default;
        virtual void load(const std::string_view path) = 0;
        virtual void bind() = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////

    struct iaudio_buffer
    {
        virtual ~iaudio_buffer() = default;

        enum class running_mode
        {
            once,
            for_ever
        };

        virtual void play(const running_mode mode) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////

    class iengine
    {
    public:
        virtual ~iengine() = default;
        virtual void init() = 0;
        virtual bool process_input(event& event) = 0;
        virtual bool key_down(const enum keys key) = 0;
        virtual void imgui_new_frame() = 0;
        virtual void imgui_render() = 0;

        /* clang-format off */
        virtual void render(ivertex_buffer* vertex_buffer,
                            i_index_buffer* ebo,    
                            itexture* const texture,
                            const glm::mediump_mat3& matrix) = 0;
        virtual void render(ivertex_buffer* vertex_buffer,
                    i_index_buffer* ebo,
                    itexture* const texture) = 0;

        virtual ivertex_buffer* create_vertex_buffer(
            const std::vector<triangle>& triangles) = 0;
        virtual ivertex_buffer* create_vertex_buffer(
            const std::vector<vertex>& vertices) = 0;
        virtual void destroy_vertex_buffer(
            ivertex_buffer* buffer) = 0;

        virtual i_index_buffer* create_ebo(
            const std::vector<uint32_t>& indices) = 0;
        virtual void destroy_ebo(i_index_buffer* buffer) = 0;

        virtual itexture* create_texture(
            const std::string_view path) = 0;
        virtual void destroy_texture(const itexture* const texture) = 0;

        virtual iaudio_buffer* create_audio_buffer(
            const std::string_view audio_file_name) = 0;
        virtual void destroy_audio_buffer(iaudio_buffer* buffer) = 0;
        /* clang-format on */

        virtual void uninit() = 0;
        virtual void imgui_uninit() = 0;
        virtual void swap_buffers() = 0;
        virtual std::pair<size_t, size_t> get_screen_resolution() const noexcept = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////

} // namespace arci

///////////////////////////////////////////////////////////////////////////////
