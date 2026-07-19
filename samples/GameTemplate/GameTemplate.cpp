#include "GameTemplate.h"

using namespace PLAMIO;

void GameTemplate::onInit(PLAMIO::Storage& storage)
{
}

PLAMIO::Game::GameState GameTemplate::onUpdate(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage, float deltaSec)
{
    return PLAMIO::Game::GameState::RUNNING;
}

bool GameTemplate::onDraw(PLAMIO::Graphics& graphics, bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty) {
    	// Skip drawing when nothing has changed.
        return false;
    }

    dirty = false;
    return true;
}

void GameTemplate::onTerminate(PLAMIO::Storage& storage)
{
}