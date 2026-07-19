#pragma once

#include "PLAMIO.h"
#include "GraphicsBase.h"
#include <string>
#include <vector>

namespace PLAMIO {

class GameCatalog
{
private:
    struct MenuGroup
    {
        std::string name;
        std::vector<Game*> games;
    };

    std::vector<Game*> games;

    // Menu data is built lazily on first access after registration.
    mutable bool menuBuilt = false;
    mutable std::vector<Game*> rootGames;
    mutable std::vector<MenuGroup> groups;

    void ensureMenuBuilt() const;

public:
    GameCatalog();
    void addGame(Game* game, GraphicsBase& graphics);

    // Original flat catalog access.
    Game* getGame(uint16_t index) const;
    uint16_t getGameCount() const;

    // Two-level menu access. Root folders are always returned before root games.
    uint16_t getRootItemCount() const;
    bool isRootItemGroup(uint16_t index) const;
    const char* getRootItemName(uint16_t index) const;
    Game* getRootItemGame(uint16_t index) const;
    uint16_t getRootItemGroupIndex(uint16_t index) const;

    uint16_t getGroupCount() const;
    const char* getGroupName(uint16_t groupIndex) const;
    uint16_t getGroupGameCount(uint16_t groupIndex) const;
    Game* getGroupGame(uint16_t groupIndex, uint16_t gameIndex) const;
};

} // namespace PLAMIO
