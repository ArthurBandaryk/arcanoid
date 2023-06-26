#include "engine.hxx"
#include "glad/glad.h"

#include "opengl-debug.hxx"
#include "opengl-shader-programm.hxx"

//
#include <SDL3/SDL.h>

#ifdef __ANDROID__
/* clang-format off */
#include <android/log.h>
/* clang-format on */
#endif

//
#include <imgui.h>
#include <imgui_impl_opengl.hxx>

#define STB_IMAGE_IMPLEMENTATION // Should be defined before including stb_image.h.

#include <stb_image.h>

//
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

///////////////////////////////////////////////////////////////////////////////

namespace arci
{

    ///////////////////////////////////////////////////////////////////////////////

    struct bind_key
    {
        std::string_view key_name {};
        SDL_KeyCode key_code;
        key_event button_pressed;
        key_event button_released;
        arci::keys key;
    };

    ///////////////////////////////////////////////////////////////////////////////

    const std::array<bind_key, 8> keys {
        bind_key {
            "left",
            SDLK_LEFT,
            key_event::left_button_pressed,
            key_event::left_button_released,
            keys::left,
        },
        bind_key {
            "right",
            SDLK_RIGHT,
            key_event::right_button_pressed,
            key_event::right_button_released,
            keys::right,
        },
        bind_key {
            "up",
            SDLK_UP,
            key_event::up_button_pressed,
            key_event::up_button_released,
            keys::up,
        },
        bind_key {
            "down",
            SDLK_DOWN,
            key_event::down_button_pressed,
            key_event::down_button_released,
            keys::down,
        },
        bind_key {
            "space",
            SDLK_SPACE,
            key_event::button1_pressed,
            key_event::button1_released,
            keys::button1,
        },
        bind_key {
            "+",
            SDLK_KP_PLUS,
            key_event::button2_pressed,
            key_event::button2_released,
            keys::magnify,
        },
        bind_key {
            "-",
            SDLK_KP_MINUS,
            key_event::button3_pressed,
            key_event::button3_released,
            keys::reduce,
        },
        bind_key {
            "escape",
            SDLK_ESCAPE,
            key_event::escape_button_pressed,
            key_event::escape_button_released,
            keys::exit,
        },
    };

    ///////////////////////////////////////////////////////////////////////////////

    triangle::triangle(const vertex& v0, const vertex& v1, const vertex& v2)
    {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;
    }

    ///////////////////////////////////////////////////////////////////////////////

    class vertex_buffer final : public ivertex_buffer
    {
    public:
        vertex_buffer(const std::vector<triangle>& triangles)
        {
            m_num_vertices = triangles.size() * 3;

            glGenVertexArrays(1, &m_vao_id);
            opengl_check();
            glGenBuffers(1, &m_vbo_id);
            opengl_check();

            bind();

            glBufferData(
                GL_ARRAY_BUFFER,
                triangles.size() * 3 * sizeof(vertex),
                triangles.data()->vertices.data(),
                GL_STATIC_DRAW);
            opengl_check();
        }

        vertex_buffer(const std::vector<vertex>& vertices)
        {
            m_num_vertices = vertices.size();

            glGenVertexArrays(1, &m_vao_id);
            opengl_check();
            glGenBuffers(1, &m_vbo_id);
            opengl_check();

            bind();

            glBufferData(GL_ARRAY_BUFFER,
                         m_num_vertices * sizeof(vertex),
                         vertices.data(),
                         GL_STATIC_DRAW);
            opengl_check();
        }

        ~vertex_buffer()
        {
            glBindVertexArray(0);
            opengl_check();
            glDeleteVertexArrays(1, &m_vao_id);
            opengl_check();
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            opengl_check();
            glDeleteBuffers(1, &m_vbo_id);
            opengl_check();
        }

        void bind() override
        {
            glBindVertexArray(m_vao_id);
            opengl_check();
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
            opengl_check();
        }

        std::size_t get_vertices_number() const override
        {
            return m_num_vertices;
        }

    private:
        GLuint m_vbo_id {};
        GLuint m_vao_id {};
        std::size_t m_num_vertices {};
    };

    class index_buffer : public i_index_buffer
    {
    public:
        index_buffer(const std::vector<uint32_t>& indices)
        {
            m_num_indices = indices.size();

            m_indices.resize(m_num_indices);
            std::copy(indices.begin(), indices.end(), m_indices.begin());

            glGenBuffers(1, &m_ebo_id);
            opengl_check();

            bind();

            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         m_num_indices * sizeof(uint32_t),
                         indices.data(),
                         GL_STATIC_DRAW);
            opengl_check();
        }

        ~index_buffer()
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            opengl_check();
            glDeleteBuffers(1, &m_ebo_id);
            opengl_check();
        }

        void bind() override
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_id);
            opengl_check();
        }

        uint32_t* data() override
        {
            return m_indices.data();
        }

        std::size_t get_indices_number() const override
        {
            return m_num_indices;
        }

    private:
        std::vector<uint32_t> m_indices {};
        GLuint m_ebo_id {};
        std::size_t m_num_indices {};
    };

    ///////////////////////////////////////////////////////////////////////////////

    class opengl_texture : public itexture
    {
    public:
        void bind() override
        {
            CHECK(m_texture_id);
            glBindTexture(GL_TEXTURE_2D, m_texture_id);
            opengl_check();
        }

        void load(const std::string_view path) override;

        std::pair<unsigned long, unsigned long> get_texture_size() const
        {
            return { m_texture_width, m_texture_height };
        }

    private:
        GLuint m_texture_id {};
        unsigned long m_texture_width {};
        unsigned long m_texture_height {};
    };

    ///////////////////////////////////////////////////////////////////////////////

    static std::mutex audio_mutex {};

    struct audio_buffer : public iaudio_buffer
    {
        Uint8* buffer { nullptr };
        Uint32 size {};
        std::size_t current_position {};
        running_mode mode { running_mode::once };
        bool is_running { false };

        audio_buffer(const std::string_view audio_file_name,
                     const SDL_AudioSpec& desired_audio_spec);

        ~audio_buffer()
        {
            SDL_free(buffer);
        }

        void play(const running_mode mode) override
        {
            std::lock_guard<std::mutex> lock { audio_mutex };
            current_position = 0;
            is_running = true;
            this->mode = mode;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////

    class engine_using_sdl final : public iengine
    {
    public:
        engine_using_sdl() = default;

        engine_using_sdl(const engine_using_sdl&) = delete;

        engine_using_sdl(engine_using_sdl&&) = delete;

        engine_using_sdl& operator=(const engine_using_sdl&) = delete;

        engine_using_sdl& operator=(engine_using_sdl&&) = delete;

        void init() override;

        bool process_input(event& event) override;

        bool key_down(const enum keys key) override;

        void imgui_new_frame() override;

        void imgui_render() override;

        void render(ivertex_buffer* vertex_buffer,
                    i_index_buffer* ebo,
                    itexture* const texture,
                    const glm::mediump_mat3& matrix) override;

        void render(ivertex_buffer* vertex_buffer,
                    i_index_buffer* ebo,
                    itexture* const texture) override;

        itexture* create_texture(const std::string_view path) override;

        void destroy_texture(const itexture* const texture) override;

        ivertex_buffer* create_vertex_buffer(
            const std::vector<triangle>& triangles) override;

        ivertex_buffer* create_vertex_buffer(
            const std::vector<vertex>& vertices) override;

        void destroy_vertex_buffer(ivertex_buffer* buffer) override;

        i_index_buffer* create_ebo(
            const std::vector<uint32_t>& indices) override;

        void destroy_ebo(i_index_buffer* buffer) override;

        iaudio_buffer* create_audio_buffer(
            const std::string_view audio_file_name) override;

        void destroy_audio_buffer(iaudio_buffer* buffer) override;

        void swap_buffers() override;

        void uninit() override;

        void imgui_uninit() override;

        std::pair<size_t, size_t>
        get_screen_resolution() const noexcept override;

        std::uint64_t get_time_since_epoch() const;

        static void sdl_audio_callback(void* userdata, Uint8* stream, int len);

    private:
        std::optional<bind_key> get_key_for_event(
            const SDL_Event& sdl_event);

        std::unique_ptr<SDL_Window, void (*)(SDL_Window*)>
            m_window { nullptr, nullptr };

        std::unique_ptr<void, int (*)(SDL_GLContext)>
            m_opengl_context { nullptr, nullptr };

        opengl_shader_program m_textured_triangle_program {};
        opengl_shader_program m_tex_no_math_program {};

        // Desired audio spec for all sounds.
        std::vector<audio_buffer*> m_sounds {};
        SDL_AudioSpec m_desired_audio_spec {};
        SDL_AudioDeviceID m_audio_device_id {};

        std::size_t m_screen_width {};
        std::size_t m_screen_height {};
        GLuint m_vbo {};
        GLuint m_vao {};
    };

    void opengl_texture::load(const std::string_view path)
    {
        std::vector<unsigned char> raw_png_image {};

        SDL_RWops* rwop = SDL_RWFromFile(path.data(), "rb");

        std::ostringstream error_on_opening {};
        error_on_opening << "Error on opening " << path << "\n";

        if (!rwop)
        {
            print_ostream_msg_and_exit(error_on_opening);
        }

        const auto bytes_to_read = rwop->size(rwop);

        CHECK(bytes_to_read != -1);

        raw_png_image.resize(bytes_to_read);

        const auto bytes_read = rwop->read(rwop,
                                           raw_png_image.data(),
                                           bytes_to_read);

        CHECK(bytes_read == bytes_to_read);

        CHECK(!rwop->close(rwop));

        int w {}, h {}, components {}, required_comps { 4 };

        stbi_set_flip_vertically_on_load(true);

        unsigned char* raw_pixels_after_decoding
            = stbi_load_from_memory(raw_png_image.data(),
                                    static_cast<int>(raw_png_image.size()),
                                    &w,
                                    &h,
                                    &components,
                                    required_comps);

        CHECK_NOTNULL(raw_pixels_after_decoding);

        m_texture_width = w;
        m_texture_height = h;

        glGenTextures(1, &m_texture_id);
        opengl_check();

        glBindTexture(GL_TEXTURE_2D, m_texture_id);
        opengl_check();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        opengl_check();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        opengl_check();

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     m_texture_width,
                     m_texture_height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     raw_pixels_after_decoding);
        opengl_check();
        glGenerateMipmap(GL_TEXTURE_2D);
        opengl_check();
    }

    audio_buffer::audio_buffer(const std::string_view audio_file_name,
                               const SDL_AudioSpec& desired_audio_spec)
    {
        SDL_RWops* rwop_ptr_file = SDL_RWFromFile(audio_file_name.data(),
                                                  "rb");
        CHECK_NOTNULL(rwop_ptr_file);

        SDL_AudioSpec audio_spec {};

        // Load the audio data of a WAVE file into memory.
        SDL_AudioSpec* music_spec = SDL_LoadWAV_RW(rwop_ptr_file,
                                                   1,
                                                   &audio_spec,
                                                   &buffer,
                                                   &size);

        CHECK(music_spec);

        if (audio_spec.freq != desired_audio_spec.freq
            || audio_spec.channels != desired_audio_spec.channels
            || audio_spec.format != desired_audio_spec.format)
        {
            Uint8* new_converted_buffer { nullptr };
            int new_length {};
            const int status = SDL_ConvertAudioSamples(audio_spec.format,
                                                       audio_spec.channels,
                                                       audio_spec.freq,
                                                       buffer,
                                                       size,
                                                       desired_audio_spec.format,
                                                       desired_audio_spec.channels,
                                                       desired_audio_spec.freq,
                                                       &new_converted_buffer,
                                                       &new_length);

            CHECK(status == 0);
            CHECK_NOTNULL(new_converted_buffer);

            SDL_free(buffer);
            buffer = new_converted_buffer;
            size = new_length;
        }
    }

    void engine_using_sdl::init()
    {
        // SDL initialization.
        CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);

        CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                                  SDL_GL_CONTEXT_DEBUG_FLAG)
              == 0);

        CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                                  SDL_GL_CONTEXT_PROFILE_ES)
              == 0);

        int opengl_major_version { 3 }, opengl_minor_version { 2 };

        CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,
                                  opengl_major_version)
              == 0);

        CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,
                                  opengl_minor_version)
              == 0);

        CHECK(SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,
                                  &opengl_major_version)
              == 0);

        CHECK(SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,
                                  &opengl_minor_version)
              == 0);

        CHECK(opengl_major_version == 3);
        CHECK(opengl_minor_version == 2);

        m_screen_width = 1024u;
        m_screen_height = 768u;

#ifdef __ANDROID__
        int num_displays {};
        const auto* list_of_displays = SDL_GetDisplays(&num_displays);

        if (list_of_displays == nullptr)
        {
            __android_log_print(ANDROID_LOG_ERROR,
                                "ARCI",
                                "Cannot load a list of displays");
            throw std::runtime_error { "Error on getting a list of displays" };
        }

        const SDL_DisplayMode* display_mode
            = SDL_GetCurrentDisplayMode(list_of_displays[num_displays - 1]);

        CHECK_NOTNULL(display_mode);

        m_screen_width = display_mode->w;
        m_screen_height = display_mode->h;
#endif

        // Window setup.
        m_window = std::unique_ptr<SDL_Window, void (*)(SDL_Window*)>(
            SDL_CreateWindow(
                "Arcanoid",
                m_screen_width,
                m_screen_height,
                SDL_WINDOW_OPENGL),
            SDL_DestroyWindow);

        CHECK_NOTNULL(m_window.get());

#ifndef __ANDROID__
        CHECK(SDL_SetWindowPosition(m_window.get(),
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED)
              == 0);
#endif

        m_opengl_context = std::unique_ptr<void, int (*)(SDL_GLContext)>(
            SDL_GL_CreateContext(m_window.get()),
            SDL_GL_DeleteContext);
        CHECK_NOTNULL(m_opengl_context.get());

        auto load_opengl_func_pointer = [](const char* func_name) -> void* {
            SDL_FunctionPointer sdl_func_pointer
                = SDL_GL_GetProcAddress(func_name);
            CHECK_NOTNULL(sdl_func_pointer);
            return reinterpret_cast<void*>(sdl_func_pointer);
        };

        CHECK(gladLoadGLES2Loader(load_opengl_func_pointer));

        glEnable(GL_DEBUG_OUTPUT);
        opengl_check();
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        opengl_check();
        glDebugMessageCallback(opengl_message_callback, nullptr);
        opengl_check();
        glDebugMessageControl(
            GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DONT_CARE,
            0,
            nullptr,
            GL_TRUE);
        opengl_check();

        m_textured_triangle_program.load_shader(GL_VERTEX_SHADER,
                                                "texture.vert");
        m_textured_triangle_program.load_shader(GL_FRAGMENT_SHADER,
                                                "texture.frag");
        m_textured_triangle_program.prepare_program();

        m_tex_no_math_program.load_shader(GL_VERTEX_SHADER,
                                          "tex-no-math.vert");
        m_tex_no_math_program.load_shader(GL_FRAGMENT_SHADER,
                                          "tex-no-math.frag");
        m_tex_no_math_program.prepare_program();

        glGenBuffers(1, &m_vbo);
        opengl_check();

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        opengl_check();

        glGenVertexArrays(1, &m_vao);
        opengl_check();
        glBindVertexArray(m_vao);
        opengl_check();

        glEnable(GL_BLEND);
        opengl_check();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        opengl_check();

        glViewport(0, 0, m_screen_width, m_screen_height);
        opengl_check();

        ImGui_ImplSdlGL3_Init(m_window.get());

        SDL_memset(&m_desired_audio_spec, 0, sizeof(m_desired_audio_spec));
        m_desired_audio_spec.freq = 48000;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        m_desired_audio_spec.format = AUDIO_F32LSB;
        m_desired_audio_spec.channels = 2;
#else
        m_desired_audio_spec.format = SDL_AUDIO_S16LSB;
        m_desired_audio_spec.channels = 1;
#endif
        m_desired_audio_spec.samples = 4096;
        m_desired_audio_spec.callback = sdl_audio_callback;
        m_desired_audio_spec.userdata = this;

        const char* default_audio_device { nullptr };

        SDL_AudioSpec returned_from_open_audio_device {};

        m_audio_device_id
            = SDL_OpenAudioDevice(default_audio_device,
                                  0,
                                  &m_desired_audio_spec,
                                  &returned_from_open_audio_device,
                                  SDL_AUDIO_ALLOW_ANY_CHANGE);

        CHECK(m_audio_device_id != 0);

        CHECK(m_desired_audio_spec.freq
              == returned_from_open_audio_device.freq);

        CHECK(m_desired_audio_spec.channels
              == returned_from_open_audio_device.channels);

        CHECK(m_desired_audio_spec.format
              == returned_from_open_audio_device.format);

        SDL_PlayAudioDevice(m_audio_device_id);
    }

    bool engine_using_sdl::process_input(event& event)
    {
        SDL_Event sdl_event {};

        std::optional<bind_key> key_for_event {};

        if (SDL_PollEvent(&sdl_event))
        {
            ImGui_ImplSdlGL3_ProcessEvent(&sdl_event);

            switch (sdl_event.type)
            {
            case SDL_EVENT_QUIT:
                event.is_quitting = true;
                break;

            case SDL_EVENT_KEY_UP:
                event.device = event_from_device::keyboard;
                key_for_event = get_key_for_event(sdl_event);
                if (key_for_event)
                {
                    event.key_info = key_for_event->button_released;
                }
                break;

            case SDL_EVENT_KEY_DOWN:
                event.device = event_from_device::keyboard;
                key_for_event = get_key_for_event(sdl_event);
                if (key_for_event)
                {
                    event.key_info = key_for_event->button_pressed;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                event.device = event_from_device::mouse;
                event.mouse_info = mouse_event { sdl_event.button.x, sdl_event.button.y };
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                break;

            case SDL_EVENT_FINGER_DOWN:
                event.device = event_from_device::touch;
                if (sdl_event.tfinger.x <= 0.5f)
                {
                    event.key_info = key_event::left_button_pressed;
                }
                else
                {
                    event.key_info = key_event::right_button_pressed;
                }
                break;

            case SDL_EVENT_FINGER_MOTION:
                event.device = event_from_device::touch;
                if (sdl_event.tfinger.x <= 0.5f)
                {
                    event.key_info = key_event::left_button_pressed;
                }
                else
                {
                    event.key_info = key_event::right_button_pressed;
                }
                break;

            case SDL_EVENT_FINGER_UP:
                event.device = event_from_device::none;
                break;

            default:
                break;
            }

            return true;
        }

        return false;
    }

    bool engine_using_sdl::key_down(const enum keys key)
    {
        const auto iter = std::find_if(
            keys.begin(),
            keys.end(),
            [key](const bind_key& bind) {
                return bind.key == key;
            });

        if (iter != keys.end())
        {
            const Uint8* state = SDL_GetKeyboardState(nullptr);
            const SDL_Scancode sdl_scan_code
                = SDL_GetScancodeFromKey(iter->key_code);
            return state[sdl_scan_code];
        }

        return false;
    }

    itexture* engine_using_sdl::create_texture(const std::string_view path)
    {
        itexture* texture = new opengl_texture {};
        texture->load(path);
        return texture;
    }

    void engine_using_sdl::destroy_texture(const itexture* const texture)
    {
        CHECK_NOTNULL(texture);
        delete texture;
    }

    ivertex_buffer* engine_using_sdl::create_vertex_buffer(
        const std::vector<triangle>& triangles)
    {
        return new vertex_buffer { triangles };
    }

    ivertex_buffer* engine_using_sdl::create_vertex_buffer(
        const std::vector<vertex>& vertices)
    {
        return new vertex_buffer { vertices };
    }

    void engine_using_sdl::destroy_vertex_buffer(ivertex_buffer* buffer)
    {
        CHECK_NOTNULL(buffer);
        delete buffer;
    }

    i_index_buffer* engine_using_sdl::create_ebo(const std::vector<uint32_t>& indices)
    {
        return new index_buffer { indices };
    }

    void engine_using_sdl::destroy_ebo(i_index_buffer* buffer)
    {
        CHECK_NOTNULL(buffer);
        delete buffer;
    }

    iaudio_buffer* engine_using_sdl::create_audio_buffer(
        const std::string_view audio_file_name)
    {
        audio_buffer* buffer = new audio_buffer {
            audio_file_name, m_desired_audio_spec
        };

        {
            std::lock_guard<std::mutex> lock { audio_mutex };
            m_sounds.push_back(buffer);
        }

        return buffer;
    }

    void engine_using_sdl::destroy_audio_buffer(iaudio_buffer* buffer)
    {
        CHECK_NOTNULL(buffer);
        delete buffer;
    }

    void engine_using_sdl::imgui_new_frame()
    {
        ImGui_ImplSdlGL3_NewFrame(m_window.get());
    }

    void engine_using_sdl::imgui_render()
    {
        ImGui::Render();
        ImGui_ImplSdlGL3_RenderDrawLists(ImGui::GetDrawData());
    }

    void engine_using_sdl::render(ivertex_buffer* vertex_buffer,
                                  i_index_buffer* ebo,
                                  itexture* const texture)
    {
        m_tex_no_math_program.apply_shader_program();

        m_tex_no_math_program.set_uniform("s_texture");

        texture->bind();
        vertex_buffer->bind();
        ebo->bind();

        glEnableVertexAttribArray(0);
        opengl_check();
        glEnableVertexAttribArray(1);
        opengl_check();
        glEnableVertexAttribArray(2);
        opengl_check();

        glVertexAttribPointer(
            0,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(vertex),
            reinterpret_cast<void*>(0));
        opengl_check();

        glVertexAttribPointer(
            1,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(vertex),
            reinterpret_cast<void*>(3 * sizeof(float)));
        opengl_check();

        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(vertex),
            reinterpret_cast<void*>(7 * sizeof(float)));
        opengl_check();

        glDrawElements(GL_TRIANGLES,
                       ebo->get_indices_number(),
                       GL_UNSIGNED_INT,
                       0);
        opengl_check();

        glDisableVertexAttribArray(0);
        opengl_check();

        glDisableVertexAttribArray(1);
        opengl_check();

        glDisableVertexAttribArray(2);
        opengl_check();
    }

    void engine_using_sdl::render(ivertex_buffer* vertex_buffer,
                                  i_index_buffer* ebo,
                                  itexture* const texture,
                                  const glm::mediump_mat3& matrix)
    {
        m_textured_triangle_program.apply_shader_program();

        m_textured_triangle_program.set_uniform("u_matrix", matrix);
        m_textured_triangle_program.set_uniform("s_texture");

        texture->bind();
        vertex_buffer->bind();
        ebo->bind();

        glEnableVertexAttribArray(0);
        opengl_check();
        glEnableVertexAttribArray(1);
        opengl_check();
        glEnableVertexAttribArray(2);
        opengl_check();

        glVertexAttribPointer(
            0,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(vertex),
            reinterpret_cast<void*>(0));
        opengl_check();

        glVertexAttribPointer(
            1,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(vertex),
            reinterpret_cast<void*>(3 * sizeof(float)));
        opengl_check();

        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(vertex),
            reinterpret_cast<void*>(7 * sizeof(float)));
        opengl_check();

        glDrawElements(GL_TRIANGLES,
                       ebo->get_indices_number(),
                       GL_UNSIGNED_INT,
                       0);
        opengl_check();

        glDisableVertexAttribArray(0);
        opengl_check();

        glDisableVertexAttribArray(1);
        opengl_check();

        glDisableVertexAttribArray(2);
        opengl_check();
    }

    void engine_using_sdl::swap_buffers()
    {
        CHECK(!SDL_GL_SwapWindow(m_window.get()));

        glClearColor(0.f, 1.f, 1.f, 1.f);
        opengl_check();
        glClear(GL_COLOR_BUFFER_BIT);
        opengl_check();
    }

    void engine_using_sdl::uninit()
    {
        CHECK(SDL_PauseAudioDevice(m_audio_device_id) == 0);
        SDL_CloseAudioDevice(m_audio_device_id);
        SDL_Quit();
    }

    void engine_using_sdl::imgui_uninit()
    {
        ImGui_ImplSdlGL3_Shutdown();
    }

    std::pair<size_t, size_t>
    engine_using_sdl::get_screen_resolution() const noexcept
    {
        return { m_screen_width, m_screen_height };
    }

    std::uint64_t engine_using_sdl::get_time_since_epoch() const
    {
        return std::chrono::system_clock::now().time_since_epoch().count();
    }

    std::optional<bind_key> engine_using_sdl::get_key_for_event(
        const SDL_Event& sdl_event)
    {
        const auto iter = std::find_if(
            keys.begin(),
            keys.end(),
            [&sdl_event](const bind_key& k) {
                return sdl_event.key.keysym.sym == k.key_code;
            });

        if (iter != keys.end())
        {
            return std::make_optional(*iter);
        }

        return {};
    }

    void engine_using_sdl::sdl_audio_callback(void* userdata,
                                              Uint8* stream,
                                              int len)
    {
        std::lock_guard<std::mutex> lock { audio_mutex };

        std::memset(stream, 0, len);

        engine_using_sdl* engine = static_cast<engine_using_sdl*>(userdata);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        constexpr int32_t AUDIO_FORMAT = SDL_AUDIO_F32LSB;
#else
        constexpr int32_t AUDIO_FORMAT = SDL_AUDIO_S16LSB;
#endif

        for (audio_buffer* sound : engine->m_sounds)
        {
            if (sound->is_running)
            {
                std::size_t stream_len { static_cast<std::size_t>(len) };

                const Uint8* start_buffer
                    = sound->buffer
                    + sound->current_position;

                const std::size_t bytes_left_in_buffer
                    = sound->size
                    - sound->current_position;

                if (bytes_left_in_buffer <= stream_len)
                {
                    SDL_MixAudioFormat(stream,
                                       start_buffer,
                                       AUDIO_FORMAT,
                                       bytes_left_in_buffer,
                                       SDL_MIX_MAXVOLUME);
                    sound->current_position += bytes_left_in_buffer;
                }
                else
                {
                    SDL_MixAudioFormat(stream,
                                       start_buffer,
                                       AUDIO_FORMAT,
                                       stream_len,
                                       SDL_MIX_MAXVOLUME);
                    sound->current_position += stream_len;
                }

                if (sound->current_position == sound->size)
                {
                    if (sound->mode == audio_buffer::running_mode::for_ever)
                    {
                        sound->current_position = 0;
                    }
                    else
                    {
                        sound->is_running = false;
                    }
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////

    class engine_instance final
    {
    public:
        engine_instance() = delete;

        engine_instance(const engine_instance&) = delete;

        engine_instance(engine_instance&&) = delete;

        static iengine* get_instance()
        {
            CHECK(!m_is_existing);
            m_is_existing = true;
            return m_engine;
        }

    private:
        static iengine* m_engine;
        static bool m_is_existing;
    };

    iengine* engine_instance::m_engine = new engine_using_sdl {};
    bool engine_instance::m_is_existing = false;

    ///////////////////////////////////////////////////////////////////////////////

    iengine* engine_create()
    {
        return engine_instance::get_instance();
    }

    ///////////////////////////////////////////////////////////////////////////////

    void engine_destroy(iengine* engine)
    {
        CHECK_NOTNULL(engine);
        delete engine;
    }

    ///////////////////////////////////////////////////////////////////////////////
} // namespace arci

///////////////////////////////////////////////////////////////////////////////
