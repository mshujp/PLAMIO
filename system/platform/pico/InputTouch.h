#pragma once

#include "TouchXPT2046.h"
#include <utility>

namespace PRUZEA {

template<class T>
class InputTouch : public T
{
private:
    TouchXPT2046 touch;

public:
    template<class... Args>
    explicit InputTouch(const InputTouchConfig& touchConfig, Args&&... args)
        : T(std::forward<Args>(args)...), touch(touchConfig)
    {
    }

    bool begin() override
    {
        const bool baseAvailable = T::begin();
        const bool touchAvailable = touch.begin();
        return baseAvailable || touchAvailable;
    }

    void end() override
    {
        touch.end();
        T::end();
    }

    void update() override
    {
        touch.update();
        T::update();
    }

    bool touched() const override { return touch.touched(); }
    bool justTouched() const override { return touch.justTouched(); }
    bool justTouchReleased() const override { return touch.justReleased(); }
    int16_t touchX() const override { return touch.x(); }
    int16_t touchY() const override { return touch.y(); }
};

} // namespace PRUZEA