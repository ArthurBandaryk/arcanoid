#pragma once

#include <helper.hxx>

#include <glad/glad.h>

#ifndef __ANDROID__
/* clang-format off */
#include <fmt/core.h>
/* clang-format on */
#else
/* clang-format off */
#include <android/log.h>
/* clang-format on */
#endif

#include <string>

///////////////////////////////////////////////////////////////////////////////

namespace arci {

    ///////////////////////////////////////////////////////////////////////////////

    static std::string source_msg_enum_to_string(const GLenum source_msg);

    static std::string type_msg_enum_to_string(const GLenum type_msg);

    static std::string severity_msg_enum_to_string(const GLenum severity_msg);

    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDebugMessageCallback.xhtml
    [[maybe_unused]] static void APIENTRY opengl_message_callback(
            GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar *message,
            [[maybe_unused]] const void *userParam) {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
            return;
        }

        CHECK(length < GL_MAX_DEBUG_MESSAGE_LENGTH);

#ifndef __ANDROID__
        fmt::print("Message id: {}\n", id);
        fmt::print(source_msg_enum_to_string(source));
        fmt::print(type_msg_enum_to_string(type));
        fmt::print(severity_msg_enum_to_string(severity));
        fmt::print(message);
#else
        const char *c_source = source_msg_enum_to_string(source).c_str();
        const char *c_type = type_msg_enum_to_string(type).c_str();
        const char *c_severity = severity_msg_enum_to_string(severity).c_str();

        __android_log_print(ANDROID_LOG_ERROR, "ARCI", "Message id: %d\n"
                                                       "%s\n%s\n%s\n%s\n",
                            id,
                            c_source,
                            c_type,
                            c_severity,
                            message);
#endif
    }

    static std::string source_msg_enum_to_string(const GLenum source_msg) {
        std::string result{"Message source: "};

        switch (source_msg) {
            case GL_DEBUG_SOURCE_API:
                result += "calls to the OpenGL API\n";
                break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                result += "calls to a window-system API\n";
                break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                result += "a compiler for a shading language\n";
                break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                result += "a third party application associated with OpenGL\n";
                break;
            case GL_DEBUG_SOURCE_APPLICATION:
                result += "a source application associated with OpenGL\n";
                break;
            case GL_DEBUG_SOURCE_OTHER:
                result += "some other source\n";
                break;
            default:
                result += "unknown source\n";
                break;
        }

        return result;
    }

    static std::string type_msg_enum_to_string(const GLenum type_msg) {
        std::string result{"Message type: "};

        switch (type_msg) {
            case GL_DEBUG_TYPE_ERROR:
                result += "an error, typically from the API\n";
                break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                result += "some behavior marked deprecated has been used\n";
                break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                result += "something has invoked undefined behavior\n";
                break;
            case GL_DEBUG_TYPE_PORTABILITY:
                result
                        += "some functionality the user relies upon is not portable\n";
                break;
            case GL_DEBUG_TYPE_PERFORMANCE:
                result += "code has triggered possible performance issues\n";
                break;
            case GL_DEBUG_TYPE_MARKER:
                result += "command stream annotation\n";
                break;
            case GL_DEBUG_TYPE_PUSH_GROUP:
                result += "group pushing\n";
                break;
            case GL_DEBUG_TYPE_POP_GROUP:
                result += "group popping\n";
                break;
            case GL_DEBUG_TYPE_OTHER:
                result += "some other type\n";
                break;
            default:
                result += "unknown type\n";
                break;
        }

        return result;
    }

    static std::string severity_msg_enum_to_string(const GLenum severity_msg) {
        std::string result{"Message severity: "};

        switch (severity_msg) {
            case GL_DEBUG_SEVERITY_HIGH:
                result += "HIGH. All OpenGL Errors, shader compilation/linking"
                          " errors, or highly-dangerous undefined behavior\n";
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                result += "MEDIUM. Major performance warnings, "
                          "shader compilation/linking warnings, "
                          "or the use of deprecated functionality\n";
                break;
            case GL_DEBUG_SEVERITY_LOW:
                result += "LOW. Redundant state change performance warning, "
                          "or unimportant undefined behavior\n";
                break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                result += "NOTIFICATION. Anything that isn't an error or"
                          " performance issue\n";
                break;
            default:
                result += "UNKNOWN\n";
                break;
        }

        return result;
    }

    inline static void opengl_check() {
        const GLenum error = glGetError();

        if (error == GL_NO_ERROR) {
            return;
        }

        switch (error) {
            case GL_INVALID_ENUM:
#ifndef __ANDROID__
                fmt::print("An unacceptable value is specified"
                           " for an enumerated argument\n");
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "An unacceptable value is specified"
                                    " for an enumerated argument\n");
#endif
                break;
            case GL_INVALID_VALUE:
#ifndef __ANDROID__
                fmt::print("A numeric argument is out of range\n");
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "A numeric argument is out of range\n");

#endif
                break;
            case GL_INVALID_OPERATION:
#ifndef __ANDROID__
                fmt::print("The specified operation is not"
                           " allowed in the current state\n");
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "The specified operation is not"
                                    " allowed in the current state\n");
#endif
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
#ifndef __ANDROID__
                fmt::print("The framebuffer object is not complete\n");
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "The framebuffer object is not complete\n");
#endif
                break;
            case GL_OUT_OF_MEMORY:
#ifndef __ANDROID__
                fmt::print("There is not enough memory left to execute"
                           " the command\n");
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "There is not enough memory left to execute"
                                    " the command\n");
#endif
                break;
            case GL_STACK_UNDERFLOW:
#ifndef __ANDROID__
                fmt::print("An attempt has been made to perform"
                           " an operation that would cause an internal"
                           " stack to underflow\n");
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "An attempt has been made to perform"
                                    " an operation that would cause an internal"
                                    " stack to underflow\n");
#endif
                break;
            case GL_STACK_OVERFLOW:
#ifndef __ANDROID__
                fmt::print("An attempt has been made to perform"
                           " an operation that would cause an internal"
                           " stack to overflow\n");
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "An attempt has been made to perform"
                                    " an operation that would cause an internal"
                                    " stack to overflow\n");
#endif
                break;
            default:
#ifndef __ANDROID__
                fmt::print("Undefined opengl error type\n");
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "Undefined opengl error type\n");
#endif
                break;
        }

#ifndef __ANDROID__
        std::exit(1);
#else
        // On Android we cannot call std::exit since it will cause
        // incorrect app finish.
        throw std::runtime_error{"Opengl issue"};
#endif
    }

    ///////////////////////////////////////////////////////////////////////////////

} // namespace arci

///////////////////////////////////////////////////////////////////////////////
