#pragma once

#include <fmt/core.h>
#include <sstream>
#include <type_traits>

namespace arci
{

#define CHECK(condition) \
    core::check_impl(#condition, (condition), (__FILE__), (__LINE__))

#define CHECK_NOTNULL(pointer) \
    core::check_notnull_impl(#pointer, (pointer), (__FILE__), (__LINE__))

    inline void print_ostream_msg_and_exit(const std::ostringstream& os)
    {
        fmt::print(os.str());
        std::exit(1);
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
                fmt::print("Check failed: {}\n"
                           "File: {}\n"
                           "Line: {}\n",
                           condition_name, file_name, line);
                std::exit(1);
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
                fmt::print("CHECK_NOTNULL(...) failed: {} is nullptr\n"
                           "File: {}\n"
                           "Line: {}\n",
                           ptr_name, file_name, line);
                std::exit(1);
            }
        }

        inline void check_notnull_impl(const char* ptr_name,
                                       [[maybe_unused]] const std::nullptr_t
                                           ptr,
                                       const char* file_name,
                                       const std::size_t line)
        {
            fmt::print("CHECK_NOTNULL(...) failed: {} is nullptr\n"
                       "File: {}\n"
                       "Line: {}\n",
                       ptr_name, file_name, line);
            std::exit(1);
        }
    }
}
