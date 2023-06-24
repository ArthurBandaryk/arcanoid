#pragma once

#ifndef __ANDROID__
/* clang-format off */
#include <fmt/core.h>
/* clang-format on */
#endif

#include <sstream>
#include <type_traits>

#ifdef __ANDROID__
/* clang-format off */
#include <android/log.h>
#include <stdexcept>
/* clang-format on */
#endif

namespace arci
{

#define CHECK(condition) \
    core::check_impl(#condition, (condition), (__FILE__), (__LINE__))

#define CHECK_NOTNULL(pointer) \
    core::check_notnull_impl(#pointer, (pointer), (__FILE__), (__LINE__))

    inline void print_ostream_msg_and_exit(const std::ostringstream& os)
    {
#ifndef __ANDROID__
        fmt::print(os.str());

        // On Android we can not call std::exit as it will finish app
        // incorrectly.
        std::exit(1);
#else
        __android_log_print(ANDROID_LOG_ERROR, "ARCI", "%s", os.str().c_str());
        throw std::runtime_error { "Runtime error" };
#endif
    }

    namespace core
    {
        inline void check_impl(const char* condition_name,
                               const bool condition,
                               const char* file_name,
                               const std::size_t line)
        {
            if (!condition)
            {
#ifndef __ANDROID__
                fmt::print("Check failed: {}\n"
                           "File: {}\n"
                           "Line: {}\n",
                           condition_name, file_name, line);
                std::exit(1);
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "Check failed: %s\nFile: %s\nLine: %d",
                                    condition_name,
                                    file_name,
                                    static_cast<int>(line));
                throw std::runtime_error { "Check failed" };
#endif
            }
        }

        template<typename T>
        inline void check_notnull_impl(const char* ptr_name,
                                       const T ptr,
                                       const char* file_name,
                                       const std::size_t line)
        {
            static_assert(std::is_pointer_v<T>,
                          "CHECK_NOTNULL(...) should take a pointer"
                          " as an argument");

            if (ptr == nullptr)
            {
#ifndef __ANDROID__
                fmt::print("CHECK_NOTNULL(...) failed: {} is nullptr\n"
                           "File: {}\n"
                           "Line: {}\n",
                           ptr_name, file_name, line);
                std::exit(1);
#else
                __android_log_print(ANDROID_LOG_ERROR,
                                    "ARCI",
                                    "CHECK_NOTNULL(...): %s\nFile: %s\nLine: %d",
                                    ptr_name,
                                    file_name,
                                    static_cast<int>(line));
                throw std::runtime_error { "Check failed" };
#endif
            }
        }

        inline void check_notnull_impl(const char* ptr_name,
                                       [[maybe_unused]] const std::nullptr_t
                                           ptr,
                                       const char* file_name,
                                       const std::size_t line)
        {
#ifndef __ANDROID__
            fmt::print("CHECK_NOTNULL(...) failed: {} is nullptr\n"
                       "File: {}\n"
                       "Line: {}\n",
                       ptr_name, file_name, line);
            std::exit(1);
#else
            __android_log_print(ANDROID_LOG_ERROR,
                                "ARCI",
                                "CHECK_NOTNULL(...): %s\nFile: %s\nLine: %d",
                                ptr_name,
                                file_name,
                                static_cast<int>(line));
            throw std::runtime_error { "Check failed" };

#endif
        }
    }
}
