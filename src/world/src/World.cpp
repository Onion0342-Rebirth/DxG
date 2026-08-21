#include "world/World.h"

namespace d25 {

void World::update(float dt) {
    for (Character& c : characters_) {
        c.update(dt, map_);
    }
}

} // namespace d25
