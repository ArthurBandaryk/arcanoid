#pragma once

#include "component.hxx"
#include "entity.hxx"

#include <string>
#include <unordered_map>

namespace arcanoid
{
    struct coordinator
    {
        std::unordered_map<entity, position> positions {};
        std::unordered_map<entity, bound> bounds {};
        std::unordered_map<entity, sprite> sprites {};
        std::unordered_map<entity, transform2d> transformations {};
        std::unordered_map<entity, key_inputs> inputs {};
        std::unordered_map<entity, collision> collidable_entities {};
        std::unordered_map<std::string, entity> collidable_ids {};
        std::unordered_map<std::string, arci::iaudio_buffer*> sounds {};

        void destroy_entity(const entity id);
    };
}
