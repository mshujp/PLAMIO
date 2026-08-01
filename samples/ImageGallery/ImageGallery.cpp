#include "ImageGallery.h"
#include "ImageAssets.h"

using namespace PRUZEA;

const char* ImageGallery::getId() const
{
    return "image_gallery";
}

const char* ImageGallery::getName() const
{
    return "Image Gallery";
}

const char* ImageGallery::getMenuName() const
{
    return "14 Image Gallery";
}

const char* ImageGallery::getMenuGroup() const
{
    return "SAMPLES";
}

uint16_t ImageGallery::getLogicalScreenWidth() const
{
    return SCREEN_W;
}

uint16_t ImageGallery::getLogicalScreenHeight() const
{
    return SCREEN_H;
}

uint16_t ImageGallery::getTargetScreenWidth() const
{
    return SCREEN_W;
}

uint16_t ImageGallery::getTargetScreenHeight() const
{
    return SCREEN_H;
}

void ImageGallery::onInit(Storage& storage)
{
    (void)storage;

    closeImages();
    characterX = 128.0f;
    characterY = 92.0f;
    characterAngle = 0.0f;
    transparencyEnabled = true;

    backgroundImage = Graphics::Image::loadJpeg(sample_background_jpg, sample_background_jpg_size, JPEG_W, JPEG_H, Graphics::Image::Fit::STRETCH);
    characterImage = Graphics::Image::loadPng(sample_character_png, sample_character_png_size, PNG_W, PNG_H, Graphics::Image::Fit::STRETCH);

    loadSucceeded = backgroundImage != nullptr && characterImage != nullptr;
    if (!loadSucceeded) closeImages();
    dirty = true;
}

Game::GameState ImageGallery::onUpdate(Input& input, Audio& audio, Storage& storage, float deltaSec)
{
    (void)storage;

    bool changed = false;
    float moveX = 0.0f;
    float moveY = 0.0f;

    if (input.pressed(Input::LEFT))
    {
        moveX -= MOVE_SPEED * deltaSec;
    }
    if (input.pressed(Input::RIGHT))
    {
        moveX += MOVE_SPEED * deltaSec;
    }
    if (input.pressed(Input::UP))
    {
        moveY -= MOVE_SPEED * deltaSec;
    }
    if (input.pressed(Input::DOWN))
    {
        moveY += MOVE_SPEED * deltaSec;
    }

    if (moveX != 0.0f || moveY != 0.0f)
    {
        characterX = Math::clamp(
            characterX + moveX,
            0.0f,
            static_cast<float>(SCREEN_W - PNG_W));
        characterY = Math::clamp(
            characterY + moveY,
            34.0f,
            static_cast<float>(224 - PNG_H));
        changed = true;
    }

    if (input.pressed(Input::X))
    {
        characterAngle += ROTATION_SPEED * deltaSec;
        while (characterAngle >= 360.0f) characterAngle -= 360.0f;
        changed = true;
    }

    if (input.justPressed(Input::A))
    {
        transparencyEnabled = !transparencyEnabled;
        audio.playSE(&Audio::SE::NO_1, 0.5f);
        changed = true;
    }

    if (input.justPressed(Input::B))
    {
        characterX = 128.0f;
        characterY = 92.0f;
        characterAngle = 0.0f;
        audio.playSE(&Audio::SE::NO_2, 0.5f);
        changed = true;
    }

    if (changed)
    {
        dirty = true;
    }

    return GameState::RUNNING;
}

bool ImageGallery::onDraw(
    Graphics& graphics,
    bool requestFullRedraw)
{
    if (!requestFullRedraw && !dirty)
    {
        return false;
    }

    drawScene(graphics);
    drawStatus(graphics);

    dirty = false;
    return true;
}

void ImageGallery::onTerminate(Storage& storage)
{
    (void)storage;
    closeImages();
}

void ImageGallery::closeImages()
{
    if (characterImage != nullptr)
    {
        characterImage->close();
        characterImage = nullptr;
    }

    if (backgroundImage != nullptr)
    {
        backgroundImage->close();
        backgroundImage = nullptr;
    }
}

void ImageGallery::drawScene(Graphics& graphics)
{
    graphics.fillScreen(Graphics::rgb565(12, 24, 38));

    graphics.fillRect(0, 34, SCREEN_W, 190, Graphics::rgb565(20, 44, 54));

    for (int16_t y = 34; y < 224; y += 24)
    {
        for (int16_t x = 0; x < SCREEN_W; x += 24)
        {
            if (((x / 24) + (y / 24)) & 1)
            {
                graphics.fillRect(x, y, 24, 24, Graphics::rgb565(24, 52, 62));
            }
        }
    }

    if (!loadSucceeded)
    {
        graphics.drawString(
            "IMAGE LOAD FAILED",
            SCREEN_W / 2,
            112,
            Graphics::RED,
            Graphics::SIZE_18,
            Graphics::HorizontalAlign::CENTER,
            Graphics::VerticalAlign::MIDDLE);
        return;
    }

    const int16_t backgroundX = (SCREEN_W - JPEG_W) / 2;
    const int16_t backgroundY = 72;

    graphics.drawRect(
        backgroundX - 2,
        backgroundY - 2,
        JPEG_W + 4,
        JPEG_H + 4,
        Graphics::WHITE);
    graphics.drawImage(*backgroundImage, backgroundX, backgroundY);

    graphics.drawSprite(
        characterImage->getBitmap(),
        static_cast<int16_t>(characterX),
        static_cast<int16_t>(characterY),
        characterImage->getWidth(),
        characterImage->getHeight(),
        {
            .angle = Math::degToRad(characterAngle),
            .transparent = transparencyEnabled,
            .transparentColor = Graphics::MAGENTA
        });
}

void ImageGallery::drawStatus(Graphics& graphics)
{
    graphics.fillRect(0, 0, SCREEN_W, 34, Graphics::rgb565(5, 15, 26));

    graphics.drawString(
        "JPEG BACKGROUND + PNG CHARACTER",
        SCREEN_W / 2,
        8,
        Graphics::CYAN,
        Graphics::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    const char* transparencyText = transparencyEnabled
        ? "A: TRANSPARENCY ON"
        : "A: TRANSPARENCY OFF";

    graphics.drawString(
        transparencyText,
        SCREEN_W / 2,
        192,
        transparencyEnabled ? Graphics::GREEN : Graphics::MAGENTA,
        Graphics::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);

    graphics.drawString(
        "D-PAD: MOVE  X: ROTATE  B: RESET",
        SCREEN_W / 2,
        205,
        Graphics::WHITE,
        Graphics::SIZE_13,
        Graphics::HorizontalAlign::CENTER,
        Graphics::VerticalAlign::TOP);
}
