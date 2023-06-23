#pragma once

#include <cstdint>

namespace arcanoid
{
    using entity = std::uint64_t;

    extern entity entities_number;

    entity create_entity() noexcept;
}
