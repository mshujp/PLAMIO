# PLAMIO Game Generation Guideline for AI

## Source of Truth
  The attached files are the only authoritative specification for this task.
  Always follow:
    1. PLAMIO.h
    2. PLAMIO_AI_GUIDELINES.md
    3. The user's game design document
  Do NOT use:
    - online documentation
    - GitHub repositories
    - cached knowledge
    - previous PLAMIO versions
    - remembered APIs
    - assumptions based on similar frameworks
  If any conflict exists, the attached files always take priority.
  Never generate code using APIs that are not declared in the attached PLAMIO.h.
  When uncertain, prefer using fewer APIs rather than inventing new ones.
  Do not attempt to improve or redesign the framework.
  Do not infer missing APIs.If a feature is not provided by PLAMIO, implement it inside the Game class.

## PLAMIO Design Philosophy
  - Game Exit: Transition to [GameState::TERMINATED] only when a strict system termination request or a final exit command occurs.
  - Keep controls simple and intuitive.
  - Prioritize clarity over data density. Prefer fewer words over smaller text.

## Game Feel
  When appropriate, consider adding juice:
    - Small sound effects.
    - Screen shake (on hits or explosions).
    - Hit effects and particles.
    - Simple animations and smooth transitions.

## Requirements (Mandatory)
  - Input Rules:
    - Never use system-reserved virtual buttons (`VOL_DOWN`, `VOL_UP`, `HOME`) for gameplay mechanics. They are reserved for the system OS.
    - Avoid  Spamming: Never call `playSE()` or `playMusic()` continuously every frame in `update()`. 
    - Trigger SFX only once when an event occurs (e.g., using `input.justPressed()`, or the exact frame damage is taken).
    - Strict File I/O: Any asset loading via the `File` class must be performed inside `init()`. Never open, read, or close files inside `update()` or `draw()`, as disk I/O degrades runtime performance.
    - Functions such as pressed(), justPressed(), held(), repeat() and ...accept exactly one Button value.
    - To detect button combinations, call these functions separately.
       Cor: pressed(Button::LEFT) && pressed(Button::A)
       Wrong: pressed(static_cast<Button>(Button::LEFT | Button::A))
  - Feedback: Give players clear visual feedback for important events (e.g., taking damage, scoring).
  - Graphics & Rendering Rules:
    - Screen Resolution: The standard visible screen boundary is strictly **320 x 240** (drawn from 0,0 within the 384x288 buffer). Ensure all gameplay elements, scores, and UI fit cleanly within this 320x240 area.
    - Large Asset Memory (RODATA): Any sprite, bitmap, or tile map data array MUST be defined as `static const uint16_t` (or `const`) so it resides in Flash memory (PROGMEM). Never declare raw, non-const image arrays inside functions or as mutable global variables, as this instantly causes Stack Overflow or Out of Memory.
  - Text Constraints: For all text, use a text scale of 0.8 or larger. Never let text extend beyond the screen boundaries or shrink to fit.
  - Feedback: Give players clear visual feedback for important events (e.g., taking damage, scoring).
  - Lifecycle rule:
      Do not call init(), update(), draw(), or terminate() from inside the game.
      These are called only by the PLAMIO system.
      If you need to reset gameplay, create a private resetGame() function.

## Recommendations (Optional but Preferred)
  - Score System: Display the current score. Consider displaying and saving a high score.
  - Pausing: Consider displaying a pause overlay.
  - Progression: If the game has stages, consider displaying a stage clear screen.
  - Damage Feedback: Consider adding visual effects like screen flash, shake, or explosion.
  - Use GameState only to report the broad lifecycle state to the system.
      For detailed internal states, define a private enum inside the Game class.
      Example: MODE_TITLE, MODE_PLAYING, MODE_GAME_OVER, MODE_STAGE_CLEAR.

## Time Management Rules
  - Do not use frame counters to measure real-world time such as seconds, countdowns, invincibility duration, result display duration, or reaction time.
  - Use millisecond-based time functions for real-world time.
  - Frame counters may be used only for visual animation patterns where exact real time is not important.

## Timing Guidelines
  - Use Platform::getMsec() for timers, delays, blinking, and scheduled events.
    ```
    const uint32_t now = Platform::getMsec();
    if (static_cast<uint32_t>(now - startMsec) >= durationMsec) {
        // elapsed
    }
    ```
  
  - For continuous movement or physics, calculate delta time when frame-rate-independent behavior is necessary.
  - Do not use frame counts for gameplay timing.
  - There are two kinds of time in games:
    - Real Time (milliseconds)
      Use millisecond-based timing for anything the player perceives as real time.
        Examples:
         - Countdowns
         - Invincibility duration
         - Cooldowns
         - Reaction time
         - Result screens
         - Timed events
    - Visual Time (frames)
      Frame counters are recommended only for visual effects where exact timing is not important.
        Examples:
          - Sprite animation
          - Cursor blinking
          - Particle lifetime
          - Background scrolling
          - Decorative effects
  - [!IMPORTANT] CRITICAL AI RULE FOR FRAMERATE AND MOVEMENT:
    - The actual framerate is NOT guaranteed to remain constant at the fixed rate specified in the "Hardware Specifications" section above.
    - For instance, when using heavy displays like "ILI9341 SPI", the framerate MAY DROP to around 20-24 FPS.
    - NEVER assume 1 frame equals exactly 1/FPS seconds (e.g., 1/30s).
    - To prevent slow-motion or physics breaking during frame drops, ALL movement, timers, and physics 
      MUST be calculated dynamically based on the target FPS specified above (or by using Delta Time if available).
    - Always write robust, frame-rate independent code that accommodates potential performance drops.
  - Fast-moving objects may require physics substeps.
    Example:
      split deltaSec into steps of 0.005f to 0.01f and perform collision detection for every substep.
      Always place an upper bound on the number of substeps.
  - Time passing alone does not request a redraw.
    While a time-based visual effect is active, set dirty = true.
    Examples:
      - blinking text
      - screen shake
      - particles
      - countdowns
      - invincibility flashing
    Stop setting dirty when the visual effect finishes.

## Graphics API
  - These primitives were added because multiple AI-generated games naturally attempted to use them without being instructed to do so.
  - System OSD Overlay:
    The system OS automatically overwrites the volume and battery level as transparent text/icons within the bottom 15 pixels (Y: 225 to 240). 
    To achieve a more polished and professional aesthetic, you MUST always render the game's background graphics completely down to the bottom edge (Y: 239);
    having the transparent system overlay sit directly on top of the live game background looks significantly cooler and more integrated. However, to avoid
    overlapping with these indicators, you must never place important game UI elements (such as text, scores, or gauges) at or below Y: 225.
  - Font selection.
    AI tends to choose fonts that are too small.
    Prefer SIZE_18 or larger unless screen space is very limited.
    When text width is required, always use getTextWidth() instead of estimating.

## Color Guidelines
  - Always store and pass colors as Graphics::Color.
  - Use predefined colors for common values.
  - Use Graphics::rgb565() for custom colors.
  - Do not use raw uint16_t color values directly unless importing image data.

## [IMPORTANT] UI text layout:
  Center alignment only positions text; it does not guarantee that the text 
  fits inside a panel or button. Measure text dimensions and reserve padding
  whenever text is drawn inside a fixed-size container.
  ```cpp
    const char* message = "ALL GEMS COLLECTED!";
    const auto font = Graphics::SIZE_18;
    const int16_t paddingX = 12;
    const int16_t paddingY = 8;
    const int16_t textW = graphics.getTextWidth(message, font);
    const int16_t textH = graphics.getTextHeight(message, font);
    const int16_t panelH = textH + paddingY * 2;
    const int16_t panelX = (SCREEN_W - panelW) / 2;
    const int16_t panelY = 82;
    graphics.fillRoundRect(panelX, panelY, panelW, panelH, 8, panelColor);
    graphics.drawString(message,SCREEN_W / 2, panelY + panelH / 2, textColor, font, Graphics::HorizontalAlign::CENTER,Graphics::VerticalAlign::MIDDLE);
  ```

## Japanese font guideline:
  - To reduce binary size, PLAMIO provides only one native Japanese font size. Japanese text is scaled internally for other sizes.
    Prefer short Japanese strings and avoid dense paragraphs on 320x240 screens.
    Japanese fonts are NOT available for every English font size.
  - Do not create names like SIZE_18J or SIZE_25J.
    Use only the defined Japanese fonts:
    - SIZE_16J
    - SIZE_20J
    - SIZE_32J

## Dirty Rendering Rules
  - Set dirty = true in update() whenever visible game state changes.
  - At the beginning of draw():
      if (!requestFullRedraw && !dirty)
      {
          return false;
      }
  - requestFullRedraw always takes priority over dirty.
  - After completing the drawing, set dirty = false and return true.
  - Do not set dirty = true every frame unless the screen actually changes every frame.

## Color Rules for OLED/Monochrome Games:
  Prefer a limited color palette.Avoid using many saturated primary colors at the same time.Use one accent color for emphasis.
  Examples: (Background: Dark gray, Text: White, Highlight: Cyan, Warning: Orange,Danger: Red)
  When developing games designed for the SSD1306 logical size (128x64), always use `Graphics::SSD1306_OFF` for background/clear and `Graphics::SSD1306_ON` for drawings.
  Do not hardcode RGB values (like Red or Blue) for monochrome targets, ensuring the game renders correctly on both physical OLEDs and scaled color LCDs.

## Audio Rulies
  - When playSE() is called multiple times before audio processing:
    - SE requests are not queued.
    - The most recent request replaces the previous request.
    - Simultaneous SE playback is not supported.

## Storage Rules
  - PLAMIO storage has two separate roles:
    UserFile:
      Writable per-game persistent storage.
      Similar to a console memory card or application registry.
      Use readUserFile() and writeUserFile() for save data, high scores,
      progress, and game settings.
    File:
      Read-only packaged resource access.
      Similar to a game disc or cartridge.
      Use openRead() for maps, scenarios, images, level data, and other
      packaged assets.
  - Never construct a save-data path manually.
  - Never use openRead() for UserFile save data.
  - Never attempt to write to File resources.
  - Files opened with openRead() must be explicitly closed after use.
  - Resource File access must occur in init(), not in update() or draw().
  - Do not write save data every frame.Mark save data as dirty when progress changes.
    Write it at safe transition points such as stage clear, game over, return to title, or onTerminate().

## Coding Rules
  - Prefer fixed-size arrays
  - Never use frame count for gameplay timing

## Preferred C/C++ Functions
  Use these common C/C++ functions in PLAMIO games.
  Do not invent helper functions when these are enough.
  - Math:
    sinf(), cosf(), fabsf(), sqrtf(), atan2f()
  - Formatting:
    snprintf()
  - String / parsing:
    strlen(), strcmp(), strncmp(), atoi(), strtoul()
  - Memory:
    memset(), memcpy()
  - Random:
    rand()
  - Time:
    Platform::getMsec()
  - [!IMPORTANT] Avoid: std::sinf(), std::cosf(), std::fabsf() itoa(), std::itoa(), std::to_string() sprintf(), mini_sprintf(), custom formatting helpers
                        std::vector, std::map, std::unordered_map dynamic allocation with new/delete
  - [!IMPORTANT] When using standard C library functions like strncmp, atoi, and snprintf, ensure you include all required headers (such as <cstring>, <cstdlib>, and <cstdio>) in the implementation file.
    Use the C library functions directly.
      Correct:
        memcpy()
        memset()
        snprintf()
      Do NOT write:
        std::memcpy()
        std::memset()
        std::snprintf()
  - Prefer lengthSquared() for distance comparisons when the actual distance value is not required, because it avoids sqrtf().

## Gameplay Tuning & Balancing Rules
  To allow human developers to easily fine-tune the game experience after generation, all gameplay balancing
  parameters (e.g., movement speeds, object dimensions, score rewards, time limits, or spawn rates) MUST be defined as centralized,
  clearly named constants at the top of the file. Do not hardcode these variables inside the gameplay logic.

## Random Number Generation
  - The PLAMIO system initializes the pseudo-random number generator　before any game starts.
  - Games should use std::rand() (or rand()) directly.
  - Games must NOT call std::srand().

## Music Guidelines
  - Prefer pentatonic melodies.
  - Use BPM between 100 and 160.
  - Prefer Quarter and Eighth notes.
  - Use REST between phrases.
  - Keep phrases short (4–8 bars).
  - Avoid large octave jumps.
  - Use repetition with slight variations.
  - Melody should remain recognizable when played as a square wave.
  - Boss Theme
    - Boss music should use a repeating rhythmic ostinato.
    - Avoid excessive long rests.
    - Prefer short 2–4 bar motifs with repetition.
    - Keep the melody active during combat.
    - Use minor pentatonic or natural minor scales.    
  - If the game has no BGM, do not call playMusic(). Never call playMusic(nullptr) unless the API explicitly documents that behavior.

## File & Include Rules
  ### Header File (<ClassName>.h): 
    - Must explicitly #include "PLAMIO.h" at the very beginning of the file to inherit the 'Game' base class and system APIs.
    - Verify the namespace: Accurately identify whether the class belongs directly under PLAMIO or within a nested namespace. (e.g., PLAMIO::Graphics is a class, NOT a namespace. Do NOT assume nested structures like PLAMIO::Graphics::Graphics).
    - Prohibit using namespace: Never use using namespace statements in header files. It pollutes the global namespace of any file that includes this header, causing unpredictable definition conflicts.
    - When you use constexpr, provide the initializer at the point of declaration in the header.
  ### Implementation File (<ClassName>.cpp): 
    - Must #include its corresponding header (e.g., #include "Game1.h").
    - Mandatory You must explicitly declare "using namespace PLAMIO;" at the top of the .cpp file to avoid repetitive prefixing and keep the code clean.
    - Do NOT #include "PLAMIO.h" directly here (it is already included via the header).
    - You CAN freely include standard C++ libraries (e.g., <cstdlib>, <cmath>) if required for game logic.
    - Verify the namespace: Ensure you absolutely avoid namespace/class name duplication or misidentification errors. Since using namespace PLAMIO; is declared, simply use Graphics&, Input&, or Audio& as function arguments. Never write incorrect nested types like PLAMIO::Graphics::Graphics&.

## Rules for Overriding Virtual Functions
  - When overriding functions (such as `onInit`, `onUpdate`, `onDraw`, etc.) in a derived class, you **MUST** double-check the base class definition in the provided header file (`PLAMIO.h`) character by character immediately before generating the code.
  - It is **strictly forbidden** to guess or assume function signatures based on common practices of general game frameworks (e.g., assuming a `void Draw()` function). 
  - You **MUST** ensure that the return type, argument types, and `const` qualifiers match the base class perfectly.

## Strict Code Generation Rules
  PLAMIO V1.x uses static linking. You must generate exactly two separate files:
  - Header File:  <ClassName>.h
    - Contains the complete class declaration, including member variables and method declarations.
    - Do not place method implementations in the header, except compiler-required constexpr or inline definitions.
  -.Implementation file <ClassName>.cpp
    - Contains the class implementation only.
  - Generation Order & Output Constraints:
      Step 1: Generate the header file (<ClassName>.h) first.
      Step 2: Generate the implementation file (<ClassName>.cpp).

## Target Environment
  - Platform: pico-sdk (C++17)
  - Standard headers such as <cmath>, <cstdlib>, and <cstdint> may be used.
  - Strict Memory Rule: Avoid dynamic memory allocation. Prefer fixed-size arrays over STL containers (e.g., std::vector).

The PLAMIO runtime provides:
  - Random seed initialization
  - Fixed frame rate
  - Input initialization
  -  initialization
  - Graphics initialization
  - Storage initialization

## Game Logic
  - Game Logic is intentionally left to the developer.
  - The SDK provides the building blocks, not the game rules.

## Workflow
  1. Discuss the game idea.
  2. Confirm the final specification with the user.
  3. Wait for explicit approval.
  4. Start implementation.

## Game sample

```h
#pragma once
#include "PLAMIO.h"

class Sample : public PLAMIO::Game
{
public:
    const char* getId() const override;
    const char* getName() const override;
    const char* getMenuName() const override;

    uint16_t getLogicalScreenWidth() const override;
    uint16_t getLogicalScreenHeight() const override;
    uint16_t getTargetScreenWidth() const override;
    uint16_t getTargetScreenHeight() const override;

protected:
    void onInit(PLAMIO::Storage& storage) override;
    Game::GameState onUpdate(PLAMIO::Input& input, PLAMIO::Audio& audio, PLAMIO::Storage& storage, float deltaSec) override;
    bool onDraw(PLAMIO::Graphics& g, bool requestFullRedraw) override;
    void onTerminate(PLAMIO::Storage& storage) override;

private:
};
```

## Feel free to propose improvements when appropriate!
