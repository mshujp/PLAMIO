cmake_minimum_required(VERSION 3.13)



###### ENVIRONMENT START ######
# Edit this block to choose the default PLAMIO hardware profile.

# Pin profile options:
#   system/platform/pico/boards/XXX.h
set(PLAMIO_PIN_CONFIG_DEFAULT "system/platform/pico/boards/RaspberryPi_Pico.h")
#set(PLAMIO_PIN_CONFIG_DEFAULT "system/platform/pico/boards/RaspberryPi_Pico2.h")
#set(PLAMIO_PIN_CONFIG_DEFAULT "system/platform/pico/boards/WaveShare_RP2040-ZERO.h")
#set(PLAMIO_PIN_CONFIG_DEFAULT "system/platform/pico/boards/WeAct_RP2350B_ILI9341SPI_I2S_SNES_SD.h")

# Board target options: RP2040, RP2350
set(PLAMIO_TARGET_DEFAULT "RP2040")
#set(PLAMIO_TARGET_DEFAULT "RP2350")

# Display options: ILI9341, SSD1306
set(PLAMIO_DISPLAY_DEFAULT "SSD1306")
#set(PLAMIO_DISPLAY_DEFAULT "ILI9341")

# Storage options: SD, NONE
set(PLAMIO_STORAGE_DEFAULT "NONE")
#set(PLAMIO_STORAGE_DEFAULT "SD")

# Audio options: I2S, PWM, NONE
set(PLAMIO_AUDIO_DEFAULT "PWM")
#set(PLAMIO_AUDIO_DEFAULT "I2S")
#set(PLAMIO_AUDIO_DEFAULT "NONE")

# Input options: GPIO_BUTTONS, SNES 
set(PLAMIO_INPUT_DEFAULT "GPIO_BUTTONS")
#set(PLAMIO_INPUT_DEFAULT "SNES")

# Japanese font options: ON, OFF
set(PLAMIO_JAPANESE_FONT_DEFAULT OFF)
#set(PLAMIO_JAPANESE_FONT_DEFAULT ON)

# PSRAM options: ON, OFF
set(PLAMIO_PSRAM_DEFAULT OFF)
#set(PLAMIO_PSRAM_DEFAULT ON)

# Sample game/tool options: ON, OFF
set(PLAMIO_SAMPLES_DEFAULT ON)
#set(PLAMIO_SAMPLES_DEFAULT OFF)

###### ENVIRONMENT END #######



set(PLAMIO_TARGET "${PLAMIO_TARGET_DEFAULT}" CACHE STRING "RP2040 or RP2350")
set_property(CACHE PLAMIO_TARGET PROPERTY STRINGS RP2040 RP2350)

set(PLAMIO_DISPLAY "${PLAMIO_DISPLAY_DEFAULT}" CACHE STRING "SSD1306 or ILI9341")
set_property(CACHE PLAMIO_DISPLAY PROPERTY STRINGS SSD1306 ILI9341)

set(PLAMIO_STORAGE "${PLAMIO_STORAGE_DEFAULT}" CACHE STRING "SD or NONE")
set_property(CACHE PLAMIO_STORAGE PROPERTY STRINGS SD NONE)

set(PLAMIO_AUDIO "${PLAMIO_AUDIO_DEFAULT}" CACHE STRING "PWM, I2S, or NONE")
set_property(CACHE PLAMIO_AUDIO PROPERTY STRINGS I2S PWM NONE)

set(PLAMIO_INPUT "${PLAMIO_INPUT_DEFAULT}" CACHE STRING "GPIO_BUTTONS, SNES or PS2")
set_property(CACHE PLAMIO_INPUT PROPERTY STRINGS GPIO_BUTTONS SNES PS2)

option(PLAMIO_JAPANESE_FONT "Enable Japanese font" ${PLAMIO_JAPANESE_FONT_DEFAULT})
option(PLAMIO_PSRAM "Enable PSRAM" ${PLAMIO_PSRAM_DEFAULT})
option(PLAMIO_SAMPLES "Build sample games and tools" ${PLAMIO_SAMPLES_DEFAULT})

set(PLAMIO_PIN_CONFIG "${PLAMIO_PIN_CONFIG_DEFAULT}" CACHE FILEPATH "Pin config header")
set(PLAMIO_PICOTOOL_PATH "" CACHE FILEPATH "Optional path to picotool executable used for automatic BOOTSEL reboot")

string(TOUPPER "${PLAMIO_TARGET}" PLAMIO_TARGET)
string(TOUPPER "${PLAMIO_DISPLAY}" PLAMIO_DISPLAY)
string(TOUPPER "${PLAMIO_STORAGE}" PLAMIO_STORAGE)
string(TOUPPER "${PLAMIO_AUDIO}" PLAMIO_AUDIO)
string(TOUPPER "${PLAMIO_INPUT}" PLAMIO_INPUT)

if (PLAMIO_TARGET STREQUAL "RP2040")
    set(PICO_BOARD pico CACHE STRING "" FORCE)
elseif (PLAMIO_TARGET STREQUAL "RP2350")
    set(PICO_BOARD pico2 CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "PLAMIO_TARGET must be RP2040 or RP2350")
endif()

foreach(option_name PLAMIO_DISPLAY PLAMIO_STORAGE PLAMIO_AUDIO PLAMIO_INPUT)
    if ("${${option_name}}" STREQUAL "")
        message(FATAL_ERROR "${option_name} must not be empty")
    endif()
endforeach()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/lib/pico-extras/external/pico_extras_import.cmake")
    set(PICO_EXTRAS_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/pico-extras)
    include(${PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
elseif (DEFINED ENV{PICO_EXTRAS_PATH})
    include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
elseif (DEFINED ENV{PICO_extras_PATH})
    include($ENV{PICO_extras_PATH}/external/pico_extras_import.cmake)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

project(PLAMIO C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(lib/LovyanGFX)
if (PLAMIO_STORAGE STREQUAL "SD")
    add_subdirectory(lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src)
endif()

set(PLAMIO_DISPLAY_SSD1306 0)
set(PLAMIO_DISPLAY_ILI9341 0)
set(PLAMIO_STORAGE_SD 0)
set(PLAMIO_STORAGE_NONE 0)
set(PLAMIO_AUDIO_I2S 0)
set(PLAMIO_AUDIO_PWM 0)
set(PLAMIO_AUDIO_NONE 0)
set(PLAMIO_INPUT_SNES 0)
set(PLAMIO_INPUT_GPIO_BUTTONS 0)
set(PLAMIO_INPUT_PS2 0)
set(PLAMIO_TARGET_RP2040 0)
set(PLAMIO_TARGET_RP2350 0)
set(PLAMIO_ENABLE_JAPANESE_FONT 0)
set(PLAMIO_ENABLE_PSRAM 0)

if (PLAMIO_DISPLAY STREQUAL "SSD1306")
    set(PLAMIO_DISPLAY_SSD1306 1)
elseif (PLAMIO_DISPLAY STREQUAL "ILI9341")
    set(PLAMIO_DISPLAY_ILI9341 1)
else()
    message(FATAL_ERROR "PLAMIO_DISPLAY must be SSD1306 or ILI9341")
endif()

if (PLAMIO_STORAGE STREQUAL "SD")
    set(PLAMIO_STORAGE_SD 1)
elseif (PLAMIO_STORAGE STREQUAL "NONE")
    set(PLAMIO_STORAGE_NONE 1)
else()
    message(FATAL_ERROR "PLAMIO_STORAGE must be SD or NONE")
endif()

if (PLAMIO_AUDIO STREQUAL "I2S")
    set(PLAMIO_AUDIO_I2S 1)
elseif (PLAMIO_AUDIO STREQUAL "PWM")
    set(PLAMIO_AUDIO_PWM 1)
elseif (PLAMIO_AUDIO STREQUAL "NONE")
    set(PLAMIO_AUDIO_NONE 1)
else()
    message(FATAL_ERROR "PLAMIO_AUDIO must be I2S, PWM, or NONE")
endif()

if (PLAMIO_INPUT STREQUAL "GPIO_BUTTONS")
    set(PLAMIO_INPUT_GPIO_BUTTONS 1)
elseif (PLAMIO_INPUT STREQUAL "SNES")
    set(PLAMIO_INPUT_SNES 1)
elseif (PLAMIO_INPUT STREQUAL "PS2")
    set(PLAMIO_INPUT_PS2 1)
else()
    message(FATAL_ERROR "PLAMIO_INPUT must be GPIO_BUTTONS, SNES or PS2")
endif()

if (PLAMIO_TARGET STREQUAL "RP2040")
    set(PLAMIO_TARGET_RP2040 1)
else()
    set(PLAMIO_TARGET_RP2350 1)
endif()

if (PLAMIO_JAPANESE_FONT)
    set(PLAMIO_ENABLE_JAPANESE_FONT 1)
endif()

if (PLAMIO_PSRAM)
    set(PLAMIO_ENABLE_PSRAM 1)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/system/PLAMIOConfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/PLAMIOConfig.h
    @ONLY
)

add_subdirectory(system)

add_custom_target(deploy_plamio_uf2
    COMMAND powershell
        -NoProfile
        -ExecutionPolicy Bypass
        -File "${CMAKE_SOURCE_DIR}/scripts/deploy-uf2.ps1"
        -Uf2Path "${CMAKE_BINARY_DIR}/system/plamio.uf2"
        -PicotoolPath "${PLAMIO_PICOTOOL_PATH}"
    DEPENDS plamio
    COMMENT "Deploying plamio.uf2"
    VERBATIM
)
