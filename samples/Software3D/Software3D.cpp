#include "Software3D.h"
#include <cstdlib>

// Tuning parameters
namespace {
    constexpr float ROTATE_SPEED = 2.5f;     // Rotation speed (radians per second equivalent)
    constexpr float CUBE_SIZE = 40.0f;       // Base size of the cube
    constexpr float CAMERA_DISTANCE = 140.0f;// Distance from camera to the cube (affects perspective intensity)
    constexpr float FOV = 200.0f;            // Field of View (focal length scale)
    constexpr uint32_t BLINK_INTERVAL = 500; // Blink interval for title text in milliseconds
}

void Software3D::resetGame() {
    // Initial tilt angles
    angleX = 0.3f;
    angleY = 0.4f;
    autoRotate = false;

    // Initialize the 8 local vertices of the cube
    localVertices[0] = { -CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE };
    localVertices[1] = {  CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE };
    localVertices[2] = {  CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE };
    localVertices[3] = { -CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE };
    localVertices[4] = { -CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE };
    localVertices[5] = {  CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE };
    localVertices[6] = {  CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE };
    localVertices[7] = { -CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE };

    // Define indices for the 6 faces (Counter-Clockwise order when viewed from outside)
    // Front face
    faceIndices[0][0] = 0; faceIndices[0][1] = 1; faceIndices[0][2] = 2; faceIndices[0][3] = 3;
    // Back face
    faceIndices[1][0] = 5; faceIndices[1][1] = 4; faceIndices[1][2] = 7; faceIndices[1][3] = 6;
    // Top face
    faceIndices[2][0] = 4; faceIndices[2][1] = 5; faceIndices[2][2] = 1; faceIndices[2][3] = 0;
    // Bottom face
    faceIndices[3][0] = 3; faceIndices[3][1] = 2; faceIndices[3][2] = 6; faceIndices[3][3] = 7;
    // Left face
    faceIndices[4][0] = 4; faceIndices[4][1] = 0; faceIndices[4][2] = 3; faceIndices[4][3] = 7;
    // Right face
    faceIndices[5][0] = 1; faceIndices[5][1] = 5; faceIndices[5][2] = 6; faceIndices[5][3] = 2;

    // Assign colorful themes to each face
    faceColors[0] = PRUZEA::Graphics::Color::RED;
    faceColors[1] = PRUZEA::Graphics::Color::GREEN;
    faceColors[2] = PRUZEA::Graphics::Color::BLUE;
    faceColors[3] = PRUZEA::Graphics::Color::YELLOW;
    faceColors[4] = PRUZEA::Graphics::Color::CYAN;
    faceColors[5] = PRUZEA::Graphics::Color::MAGENTA;
}

void Software3D::onInit(PRUZEA::Storage& storage) {
    currentMode = MODE_TITLE;
    lastBlinkTime = PRUZEA::Platform::getMsec();
    showPressStart = true;
    resetGame();
}

PRUZEA::Game::GameState Software3D::onUpdate(PRUZEA::Input& input, PRUZEA::Audio& audio, PRUZEA::Storage& storage, float deltaSec) {
    uint32_t currentTime = PRUZEA::Platform::getMsec();

    switch (currentMode) {
        case MODE_TITLE:
            // Text blinking logic based on Real Time (per framework guidelines)
            if (currentTime - lastBlinkTime >= BLINK_INTERVAL) {
                showPressStart = !showPressStart;
                lastBlinkTime = currentTime;
                dirty = true;
            }

            // Start playing when START button is pressed
            if (input.justPressed(PRUZEA::Input::START)) {
                audio.playSE(&PRUZEA::Audio::SE::NO_1, 1.0f); // Confirmation sound
                currentMode = MODE_PLAYING;
                dirty = true;
            }
            break;

        case MODE_PLAYING:
            bool isRotating = false;
            // Virtual delta time assuming 30fps target
            float dt = 1.0f / 30.0f;

            if (input.justPressed(PRUZEA::Input::A)) {
                autoRotate = !autoRotate;
                audio.playSE(&PRUZEA::Audio::SE::NO_1, 0.7f);
                dirty = true;
            }

            if (input.repeat(PRUZEA::Input::X)) {
                zoom += 0.02f;
                if (zoom > 3.0f) zoom = 3.0f;
                dirty = true;
            }
            if (input.repeat(PRUZEA::Input::Y)) {
                zoom -= 0.02f;
                if (zoom < 0.3f) zoom = 0.3f;
                dirty = true;
            }

            if (autoRotate) {
                angleY += ROTATE_SPEED * 0.55f * dt;
                angleX += ROTATE_SPEED * 0.20f * dt;
                isRotating = true;
            }

            if (input.pressed(PRUZEA::Input::B)) {
                if (input.pressed(PRUZEA::Input::UP)) {
                    offsetY += 3;
                    if (offsetY > 100) offsetY = 100;
                    dirty = true;
                }
                if (input.pressed(PRUZEA::Input::DOWN)) {
                    offsetY -= 3;
                    if (offsetY < -100) offsetY = -100;
                    dirty = true;
                }
                if (input.pressed(PRUZEA::Input::LEFT)) {
                    offsetX += 3;
                    if (offsetX > 100) offsetX = 100;
                    dirty = true;
                }
                if (input.pressed(PRUZEA::Input::RIGHT)) {
                    offsetX -= 3;
                    if (offsetX < -100) offsetX = -100;
                    dirty = true;
                }
             } else {
                // Handle D-pad inputs for rotation
                if (input.pressed(PRUZEA::Input::UP)) {
                    angleX -= ROTATE_SPEED * dt;
                    isRotating = true;
                }
                if (input.pressed(PRUZEA::Input::DOWN)) {
                    angleX += ROTATE_SPEED * dt;
                    isRotating = true;
                }
                if (input.pressed(PRUZEA::Input::LEFT)) {
                    angleY -= ROTATE_SPEED * dt;
                    isRotating = true;
                }
                if (input.pressed(PRUZEA::Input::RIGHT)) {
                    angleY += ROTATE_SPEED * dt;
                    isRotating = true;
                }
            }

            // Audio feedback during manual rotation is throttled to avoid clogging channels.
            if (isRotating) {
                dirty = true;
                const bool manualRotation =
                    input.pressed(PRUZEA::Input::UP) ||
                    input.pressed(PRUZEA::Input::DOWN) ||
                    input.pressed(PRUZEA::Input::LEFT) ||
                    input.pressed(PRUZEA::Input::RIGHT);
                if (manualRotation && rand() % 8 == 0) {
                    audio.playSE(&PRUZEA::Audio::SE::NO_5, 0.4f);
                }
            }

            // Return to title when SELECT is pressed
            if (input.justPressed(PRUZEA::Input::SELECT)) {
                audio.playSE(&PRUZEA::Audio::SE::NO_2, 1.0f); // Cancel sound
                currentMode = MODE_TITLE;
                resetGame();
                dirty = true;
            }
            break;
    }

    return PRUZEA::Game::GameState::RUNNING;
}

bool Software3D::onDraw(PRUZEA::Graphics& graphics, bool requestFullRedraw) {
    // Skip rendering if no update is required
    if (!requestFullRedraw && !dirty) {
        return false;
    }

    // Clear screen with a retro arcade dark-gray color
    graphics.fillScreen(PRUZEA::Graphics::Color::DARKGRAY);

    // Common UI Header
    graphics.resetCamera();
    graphics.drawString("3D CUBE SOFTWARE RENDERER", 160, 15,
                        PRUZEA::Graphics::Color::WHITE, PRUZEA::Graphics::SIZE_13,
                        PRUZEA::Graphics::HorizontalAlign::CENTER, PRUZEA::Graphics::VerticalAlign::MIDDLE);

    if (currentMode == MODE_TITLE) {
        // Render Title Screen
        graphics.drawString("PRESS START", 160, 120,
                            showPressStart ? PRUZEA::Graphics::Color::YELLOW : PRUZEA::Graphics::Color::DARKGRAY,
                            PRUZEA::Graphics::SIZE_25B,
                            PRUZEA::Graphics::HorizontalAlign::CENTER, PRUZEA::Graphics::VerticalAlign::MIDDLE);

        graphics.drawString("D-PAD: ROTATE   A: AUTO ROTATE", 160, 180,
                            PRUZEA::Graphics::Color::LIGHTGRAY, PRUZEA::Graphics::SIZE_13,
                            PRUZEA::Graphics::HorizontalAlign::CENTER, PRUZEA::Graphics::VerticalAlign::MIDDLE);
    }
    else if (currentMode == MODE_PLAYING) {
        // --- Start of 3D Pipeline ---
        Vector3D transformedVertices[VERTEX_COUNT];
        Point2D projectedPoints[VERTEX_COUNT];

        if (zoom != 1.0f || offsetX != 0 || offsetY != 0) {
            graphics.setCamera(
                {
                    .x = offsetX,
                    .y = offsetY,
                    .zoom = zoom,
                    .zoomCenterX = static_cast<int16_t>(getLogicalScreenWidth() / 2),
                    .zoomCenterY = static_cast<int16_t>(getLogicalScreenHeight() / 2)
                }
            );
        }

        // 1. Precompute sine and cosine values
        float sinX = sinf(angleX); float cosX = cosf(angleX);
        float sinY = sinf(angleY); float cosY = cosf(angleY);

        // 2. Vertex Transformation & Perspective Projection
        for (int i = 0; i < VERTEX_COUNT; ++i) {
            // Y-axis rotation
            float x1 = localVertices[i].x * cosY - localVertices[i].z * sinY;
            float z1 = localVertices[i].x * sinY + localVertices[i].z * cosY;

            // X-axis rotation
            float y2 = localVertices[i].y * cosX - z1 * sinX;
            float z2 = localVertices[i].y * sinX + z1 * cosX;

            // Translate along Z-axis by camera distance
            float transX = x1;
            float transY = y2;
            float transZ = z2 + CAMERA_DISTANCE;

            transformedVertices[i] = { transX, transY, transZ };

            // Perspective Projection (3D -> 2D) offset to screen center (160, 120)
            projectedPoints[i].x = static_cast<int16_t>(160 + (transX * FOV) / transZ);
            projectedPoints[i].y = static_cast<int16_t>(120 + (transY * FOV) / transZ);
        }

        // 3. Back-face Culling and Face Rendering
        for (int i = 0; i < FACE_COUNT; ++i) {
            // Retrieve vertex indices for the current face
            int idx0 = faceIndices[i][0];
            int idx1 = faceIndices[i][1];
            int idx2 = faceIndices[i][2];
            int idx3 = faceIndices[i][3];

            // Compute 2D cross product on the screen to determine winding order (CW vs CCW)
            // This easily discards faces looking away from the camera (Back-face Culling)
            int32_t vecAX = projectedPoints[idx1].x - projectedPoints[idx0].x;
            int32_t vecAY = projectedPoints[idx1].y - projectedPoints[idx0].y;
            int32_t vecBX = projectedPoints[idx2].x - projectedPoints[idx0].x;
            int32_t vecBY = projectedPoints[idx2].y - projectedPoints[idx0].y;

            int32_t crossProduct = vecAX * vecBY - vecAY * vecBX;

            // Render only if the cross product is positive (Counter-Clockwise face visibility)
            if (crossProduct > 0) {
                // Split quad into 2 triangles for solid polygon filling (Flat Shading)
                graphics.fillTriangle(projectedPoints[idx0].x, projectedPoints[idx0].y,
                                      projectedPoints[idx1].x, projectedPoints[idx1].y,
                                      projectedPoints[idx2].x, projectedPoints[idx2].y, faceColors[i]);

                graphics.fillTriangle(projectedPoints[idx0].x, projectedPoints[idx0].y,
                                      projectedPoints[idx2].x, projectedPoints[idx2].y,
                                      projectedPoints[idx3].x, projectedPoints[idx3].y, faceColors[i]);

                // Draw black outlines to distinguish edges clearly (classic retro gaming style)
                graphics.drawLine(projectedPoints[idx0].x, projectedPoints[idx0].y, projectedPoints[idx1].x, projectedPoints[idx1].y, PRUZEA::Graphics::Color::BLACK);
                graphics.drawLine(projectedPoints[idx1].x, projectedPoints[idx1].y, projectedPoints[idx2].x, projectedPoints[idx2].y, PRUZEA::Graphics::Color::BLACK);
                graphics.drawLine(projectedPoints[idx2].x, projectedPoints[idx2].y, projectedPoints[idx3].x, projectedPoints[idx3].y, PRUZEA::Graphics::Color::BLACK);
                graphics.drawLine(projectedPoints[idx3].x, projectedPoints[idx3].y, projectedPoints[idx0].x, projectedPoints[idx0].y, PRUZEA::Graphics::Color::BLACK);
            }
        }

        // Navigation guide (Placed to avoid overlaying the system OSD area at Y:225-240)
        graphics.resetCamera();
        graphics.drawString("X / Y: Zoom   B + D-Pad: Move Cube",
                            160, 200,
                            PRUZEA::Graphics::Color::LIGHTGRAY, PRUZEA::Graphics::SIZE_13,
                            PRUZEA::Graphics::HorizontalAlign::CENTER, PRUZEA::Graphics::VerticalAlign::MIDDLE);
         graphics.drawString(autoRotate ? "A: AUTO ON   SELECT: MENU"
                                       : "A: AUTO OFF  SELECT: MENU",
                            160, 215,
                            PRUZEA::Graphics::Color::LIGHTGRAY, PRUZEA::Graphics::SIZE_13,
                            PRUZEA::Graphics::HorizontalAlign::CENTER, PRUZEA::Graphics::VerticalAlign::MIDDLE);
    }

    dirty = false; // Reset the dirty flag
    return true;
}

void Software3D::onTerminate(PRUZEA::Storage& storage) {
}