#pragma once

#include "component.hxx"
#include "entity.hxx"

#include <map>
#include <string>

namespace arcanoid
{
    struct coordinator
    {
        std::map<entity, position> positions {};
        std::map<entity, sprite> sprites {};
        std::map<entity, transform2d> transformations {};
        std::map<entity, key_inputs> inputs {};
        std::map<entity, collision> collidable_entities {};
        std::map<std::string, entity> collidable_ids {};
        std::map<std::string, arci::iaudio_buffer*> sounds {};

        void destroy_entity(const entity id);
    };
}
