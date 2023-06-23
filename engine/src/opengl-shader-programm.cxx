#include "opengl-shader-programm.hxx"
#include "opengl-debug.hxx"

#include "helper.hxx"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

///////////////////////////////////////////////////////////////////////////////

namespace arci
{

    ///////////////////////////////////////////////////////////////////////////////

    opengl_shader_program::~opengl_shader_program()
    {
        std::for_each(
            m_shaders.begin(),
            m_shaders.end(),
            [](const GLuint shader_id) {
                glDeleteShader(shader_id);
                opengl_check();
            });

        glDeleteProgram(m_program);
        opengl_check();
    }

    void opengl_shader_program::load_shader(
        const GLenum shader_type,
        std::string_view shader_name)
    {
        GLuint shader_id = glCreateShader(shader_type);
        opengl_check();

        CHECK(shader_id);

        std::string path("engine/shaders/");
        path.append(shader_name);
        const std::string shader_code_string = get_shader_code_from_file(path);
        const char* shader_code = shader_code_string.data();

        glShaderSource(shader_id, 1, &shader_code, nullptr);
        opengl_check();

        glCompileShader(shader_id);
        opengl_check();

        GLint shader_compiled {};
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_compiled);
        opengl_check();

        if (!shader_compiled)
        {
            GLint log_length {};
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
            opengl_check();

            if (log_length > 1)
            {
                std::string log {};
                log.resize(log_length);
                glGetShaderInfoLog(shader_id, log_length, nullptr, log.data());
                opengl_check();
                fmt::print(log);
            }

            glDeleteShader(shader_id);
            opengl_check();
            throw std::runtime_error { "Error on compiling shader" };
        }

        m_shaders.push_back(shader_id);
    }

    void opengl_shader_program::apply_shader_program()
    {
        glUseProgram(m_program);
        opengl_check();
    }

    void opengl_shader_program::set_uniform(
        const std::string_view matrix_attribute_name,
        const glm::mediump_mat3& result_matrix)
    {
        const GLint uniform_location = glGetUniformLocation(
            m_program,
            static_cast<const GLchar*>(matrix_attribute_name.data()));
        opengl_check();
        CHECK(uniform_location != -1);

        float m[9] = {
            result_matrix[0][0],
            result_matrix[0][1],
            result_matrix[0][2],
            result_matrix[1][0],
            result_matrix[1][1],
            result_matrix[1][2],
            result_matrix[2][0],
            result_matrix[2][1],
            result_matrix[2][2],
        };

        glUniformMatrix3fv(uniform_location, 1, GL_FALSE, m);
        opengl_check();
    }

    void opengl_shader_program::set_uniform(
        const std::string_view texture_attribute_name)
    {
        const GLint location = glGetUniformLocation(
            m_program,
            texture_attribute_name.data());
        opengl_check();

        CHECK(location != -1);

        const GLint texture_unit { 0 };
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        opengl_check();
        glUniform1i(location, 0 + texture_unit);
        opengl_check();
    }

    void opengl_shader_program::prepare_program()
    {
        attach_shaders();
        link_program();
        validate_program();
        CHECK(m_program);
    }

    void opengl_shader_program::attach_shaders()
    {
        m_program = glCreateProgram();
        opengl_check();
        CHECK(m_program);

        std::for_each(
            m_shaders.begin(),
            m_shaders.end(),
            [this](const GLuint shader_id) {
                glAttachShader(m_program, shader_id);
                opengl_check();
            });
    }

    void opengl_shader_program::link_program() const
    {
        glLinkProgram(m_program);
        opengl_check();

        GLint linked {};
        glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
        opengl_check();

        if (!linked)
        {
            GLint log_length {};
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &log_length);
            opengl_check();

            if (log_length > 1)
            {
                std::string log {};
                log.resize(log_length);
                glGetProgramInfoLog(m_program, log_length, nullptr, log.data());
                opengl_check();
                fmt::print(log);
            }

            glDeleteProgram(m_program);
            opengl_check();
            throw std::runtime_error { "Error on linking shader program" };
        }
    }

    void opengl_shader_program::validate_program() const
    {
        glValidateProgram(m_program);
        opengl_check();

        GLint is_validated {};
        glGetProgramiv(m_program, GL_VALIDATE_STATUS, &is_validated);
        opengl_check();

        if (!is_validated)
        {
            GLint log_length {};
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &log_length);
            opengl_check();

            if (log_length > 1)
            {
                std::string log {};
                log.resize(log_length);
                glGetProgramInfoLog(m_program, log_length, nullptr, log.data());
                opengl_check();
                fmt::print(log);
            }

            glDeleteProgram(m_program);
            throw std::runtime_error { "Error on validating shader program" };
        }
    }

    std::string opengl_shader_program::get_shader_code_from_file(
        const std::string_view path) const
    {
        std::string shader_code {};

        std::ifstream file { path.data(), std::ios::binary };

        std::ostringstream error_on_opening {};
        error_on_opening << "Error on opening " << path << "\n";

        if (!file.is_open())
        {
            print_ostream_msg_and_exit(error_on_opening);
        }

        const std::filesystem::path fs_path { path.data() };

        std::streamsize bytes_to_read = std::filesystem::file_size(fs_path);

        CHECK(bytes_to_read);
        shader_code.resize(bytes_to_read);

        file.read(shader_code.data(), bytes_to_read);
        std::ostringstream error_on_reading {};
        error_on_reading << "Error on reading " << path << "\n";
        if (!file.good())
        {
            print_ostream_msg_and_exit(error_on_reading);
        }

        file.close();
        std::ostringstream error_on_closing {};
        error_on_closing << "Error on closing " << path << "\n";
        if (!file.good())
        {
            print_ostream_msg_and_exit(error_on_closing);
        }

        return shader_code;
    }

    ///////////////////////////////////////////////////////////////////////////////

} // namespace arci

///////////////////////////////////////////////////////////////////////////////
