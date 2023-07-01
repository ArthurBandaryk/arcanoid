#include "coordinator.hxx"

namespace arcanoid
{
    void coordinator::destroy_entity(const entity id)
    {
        positions.erase(id);
        bounds.erase(id);
        sprites.erase(id);
        transformations.erase(id);
        inputs.erase(id);
        collidable_entities.erase(id);

        for (const auto& [str, c_id] : collidable_ids)
        {
            if (id == c_id)
            {
                collidable_ids.erase(str);
                break;
            }
        }
    }
}
