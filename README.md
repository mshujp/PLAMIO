# PLAMIO

> **AI-Friendly Game Framework**

A lightweight game framework designed for AI-assisted game development.

------------------------------------------------------------------------

# Features

-   AI-friendly public API
-   Portable game code across supported platforms
-   Unified Graphics / Input / Audio / Storage APIs
-   Fixed 30 FPS game loop
-   Built-in SaveData helper
-   Camera & viewport support
-   PWM / I2S audio support
-   SSD1306 / ILI9341 display support
-   AI-oriented documentation and API design

- **Supported platforms**
  - Raspberry Pi Pico family (RP2040 / RP2350)
  - ESP32 (planned)
  
| ![](docs/images/01.jpg) | ![](docs/images/02.jpg) |
| ![](docs/images/03.jpg) | ![](docs/images/04.jpg) |

| ![](docs/images/ss01.png) | ![](docs/images/ss02.png) |
| ![](docs/images/ss03.png) | ![](docs/images/ss04.png) |
| ![](docs/images/ss05.png) | ![](docs/images/ss06.png) |
| ![](docs/images/ss07.png) | ![](docs/images/ss08.png) |
| ![](docs/images/ss09.png) | ![](docs/images/ss10.png) |
| ![](docs/images/ss11.png) | ![](docs/images/ss12.png) |
| ![](docs/images/ss13.png) | ![](docs/images/ss14.png) |
| ![](docs/images/ss15.png) | ![](docs/images/ss16.png) | 
| ![](docs/images/ss17.png) | ![](docs/images/ss18.png) |
| ![](docs/images/ss19.png) | ![](docs/images/ss20.png) |
| ![](docs/images/ss21.png) |

------------------------------------------------------------------------

# Philosophy

PLAMIO is designed so that both humans and AI can write games using the
same simple API.

Games implement only a small set of interfaces while the runtime manages
graphics, input, audio, storage, and the game loop.

This allows game logic to remain clean, portable, and easy to generate.

------------------------------------------------------------------------

# Sample Games

  Sample                    Description
  ------------------------- ------------------------
  01 PLAMIO API             Learn the basic API
  02 Collision Lab          Collision detection
  03 SoundTile              Audio and input
  04--06 Graphics Effects   Animation techniques
  07 Sprite Adventure       Sprite rendering
  08--10 3D Samples         Advanced rendering
  11 SL                     Bonus sample
  GameTemplate              Empty project template

## Learning Path

The samples are intended to be completed in numerical order.
Each sample introduces one or more new concepts while building on previous examples.

------------------------------------------------------------------------

# Build Requirements

## Required tools

-   Raspberry Pi Pico SDK
-   CMake
-   Ninja
-   Arm GNU Toolchain

------------------------------------------------------------------------

# Required Libraries
  
- [LovyanGFX](https://github.com/lovyan03/LovyanGFX)
- [pico-extras](https://github.com/raspberrypi/pico-extras) (required for I2S audio)
- [pico_audio_i2s_32b](https://github.com/elehobica/pico_audio_i2s_32b) (required for I2S audio)
- [no-OS-FatFS-SD-SDIO-SPI-RPi-Pico](https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico) (required for SD storage)  

``` text
PLAMIO/
├── games/
├── system/
└── lib/
    ├── LovyanGFX/
    ├── pico-extras/
    ├── pico_audio_i2s_32b/
    └── no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/
```

------------------------------------------------------------------------

# Configuration

## Hardware Configuration

PLAMIO uses hardware profiles to describe the complete hardware configuration of a board.

Select the hardware profile in the `###### ENVIRONMENT START ######` section of `CMakeLists.txt`.

```cmake
set(PLAMIO_PIN_CONFIG_DEFAULT "system/platform/pico/boards/WaveShare_RP2040-ZERO.h")
```

Hardware profiles are stored in:

```text
system/platform/pico/boards/
```

Each profile defines the board-specific hardware settings, including graphics, input, audio, storage, battery, and pin assignments.

To support a new board, create a new hardware profile in this directory and select it in `CMakeLists.txt`.

## Project Configuration

The `###### ENVIRONMENT START ######` section also defines the project's default configuration.

Available options include:

- Target board (`RP2040`, `RP2350`)
- Display (`ILI9341`, `SSD1306`)
- Storage (`SD`, `NONE`)
- Audio (`PWM`, `I2S`, `NONE`)
- Input (`GPIO_BUTTONS`, `SNES`)
- Japanese font (`ON`, `OFF`)
- PSRAM (`ON`, `OFF`)
- Sample projects (`ON`, `OFF`)

For example:

```cmake
set(PLAMIO_TARGET_DEFAULT "RP2040")
set(PLAMIO_DISPLAY_DEFAULT "ILI9341")
set(PLAMIO_AUDIO_DEFAULT "PWM")
```

These values define the project's default configuration and can be overridden from the command line using CMake options.

------------------------------------------------------------------------

## Build

Build the project with CMake:

```sh
cmake -S . -B build
cmake --build build
```
------------------------------------------------------------------------

## Deployment

After building, the generated firmware can be found at:

```text
build/system/plamio.uf2
```

Copy the UF2 file to a board in BOOTSEL mode to install the firmware.

VSCode tasks or custom scripts can also be used to automate the deployment process.

------------------------------------------------------------------------

# Creating a Game

## Project Structure

Each game is placed under the `games` directory.

The directory name and the game class name must match.

```text
games/
└── MyGame/
    ├── MyGame.h
    └── MyGame.cpp
```

After adding a new game, reconfigure CMake and build the project.

## Game Lifecycle

Implement only the following methods:

- `onInit()`
- `onUpdate()`
- `onDraw()`
- `onTerminate()`

PLAMIO manages the game loop and provides abstracted `Graphics`, `Input`, `Audio`, and `Storage` objects.

Game code does not need to access platform-specific hardware or drivers directly.

## AI Workflow

PLAMIO is designed for AI-assisted game development.

1. Edit `sdk/PLAMIO_GAME_DESIGN_TEMPLATE.md` to describe your game.
2. Provide the following SDK files to your AI assistant:
   - `sdk/PLAMIO.h`
   - `sdk/PLAMIO_AI_GUIDELINES.md`
   - `sdk/PLAMIO_GAME_DESIGN_TEMPLATE.md`
3. Discuss the game design with the AI.
4. Let the AI generate the game source files.
5. Add the generated files to the `games` directory, reconfigure CMake, and build the project.

------------------------------------------------------------------------

## Recommended AI

PLAMIO is designed to work with modern AI coding assistants.

Based on current development experience:

| AI | Recommendation | Notes |
|----|---------------|-------|
| **ChatGPT** | **Highly Recommended** | Best overall experience with PLAMIO |
| **Gemini** | **Recommended** | Works well for most tasks |
| **Copilot** | **Best for code completion** | Less suitable for full game generation |

------------------------------------------------------------------------

# Hardware Notes

## SD Card SPI

For SD card builds, the following configuration is recommended and has been verified on both RP2040 and RP2350.

| Peripheral | SPI |
|------------|-----|
| ILI9341 LCD | SPI1 |
| SD Card | SPI0 |

This configuration has been verified on both RP2040 and RP2350 and is recommended for best compatibility.

### SD Card

> [!IMPORTANT]
> PLAMIO supports **SDHC** and **SDXC** memory cards.
> Standard **SD cards (2GB and smaller)** are **not supported**.

> [!WARNING]
> Although PLAMIO provides a software shutdown option, embedded systems can still lose power unexpectedly (for example, due to battery removal or depletion).
> Do **not** store important or irreplaceable data on the SD card.

## PWM Audio

PWM audio supports only **MUTE** or **ON**.
If adjustable volume is required, use an external amplifier or a potentiometer.

------------------------------------------------------------------------

# Project Layout

``` text
PLAMIO/
├── sdk/
├── games/
├── system/
├── samples/
├── scripts/
├── lib/
└── ...
```

------------------------------------------------------------------------

## Supported Hardware

The following hardware configurations have been verified with PLAMIO.

| Target | Display | Input | Audio | Storage | Status |
|--------|---------|-------|-------|---------|--------|
| RP2040 | SSD1306 | GPIO Buttons | PWM | SD | ✅ Verified |
| RP2040 | ILI9341 | GPIO Buttons | PWM | SD | ✅ Verified |
| RP2350 | ILI9341 | SNES | I2S | SD | ✅ Verified |


Additional hardware configurations can be supported by creating a new hardware profile under:

```text
system/platform/pico/boards/
```

Only the configurations listed above have been verified.

------------------------------------------------------------------------

# License

MIT License
