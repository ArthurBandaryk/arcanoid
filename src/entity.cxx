#include "entity.hxx"

namespace arcanoid
{

    entity entities_number { 0 };

    entity create_entity() noexcept
    {
        static entity id { 0 };
        ++id;
        entities_number = id;
        return id;
    }
}
