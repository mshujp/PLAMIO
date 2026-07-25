#include "GameTemplate.h"

using namespace PRUZEA;

void GameTemplate::onInit(PRUZEA::Storage& storage)
{
}

PRUZEA::Game::GameState GameTemplate::onUpdate(PRUZEA::Input& input, PRUZEA::Audio& audio, PRUZEA::Storage& storage, float deltaSec)
{
    return PRUZEA::Game::GameState::RUNNING;
}

bool GameTemplate::onDraw(PRUZEA::Graphics& graphics, bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty) {
    	// Skip drawing when nothing has changed.
        return false;
    }

    dirty = false;
    return true;
}

void GameTemplate::onTerminate(PRUZEA::Storage& storage)
{
}