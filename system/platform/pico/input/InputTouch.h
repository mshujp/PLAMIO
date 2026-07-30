#pragma once

#include "graphics/lgfx/LGFXContext.h"
#include <utility>

namespace PRUZEA {

template<class T>
class InputTouch : public T
{
private:
    LGFXContext& context;
    bool currentTouched = false;
    bool previousTouched = false;
    int16_t currentX = -1;
    int16_t currentY = -1;

public:
    template<class... Args>
    explicit InputTouch(LGFXContext& context, const InputTouchConfig& touchConfig, Args&&... args)
        : T(std::forward<Args>(args)...), context(context)
    {
        context.enableTouch(touchConfig);
    }

    bool begin() override
    {
        const bool baseAvailable = T::begin();
        const bool touchAvailable = context.isTouchAvailable();
        return baseAvailable || touchAvailable;
    }

    void end() override
    {
        currentTouched = false;
        previousTouched = false;
        currentX = -1;
        currentY = -1;
        T::end();
    }

    void update() override
    {
        T::update();
        previousTouched = currentTouched;

        int16_t x = -1;
        int16_t y = -1;
        currentTouched = context.readTouch(x, y);
        currentX = currentTouched ? x : -1;
        currentY = currentTouched ? y : -1;
    }

    bool touched() const override { return currentTouched; }
    bool justTouched() const override { return currentTouched && !previousTouched; }
    bool justTouchReleased() const override { return !currentTouched && previousTouched; }
    int16_t touchX() const override { return currentX; }
    int16_t touchY() const override { return currentY; }
};

} // namespace PRUZEA
