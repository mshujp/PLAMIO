cmake_minimum_required(VERSION 3.13)



###### ENVIRONMENT START ######
# Edit this block to choose the default PRUZEA hardware profile.

# Pin profile options:
#   system/platform/pico/boards/XXX.h
set(PRUZEA_PIN_CONFIG_DEFAULT "system/platform/pico/boards/RaspberryPi_Pico.h")
#set(PRUZEA_PIN_CONFIG_DEFAULT "system/platform/pico/boards/RaspberryPi_Pico2.h")
#set(PRUZEA_PIN_CONFIG_DEFAULT "system/platform/pico/boards/WaveShare_RP2040-ZERO.h")
#set(PRUZEA_PIN_CONFIG_DEFAULT "system/platform/pico/boards/WeAct_RP2350B_ILI9341SPI_I2S_SNES_SD.h")

# Board target options: RP2040, RP2350
set(PRUZEA_TARGET_DEFAULT "RP2040")
#set(PRUZEA_TARGET_DEFAULT "RP2350")

# Display options: ILI9341, SSD1306
set(PRUZEA_DISPLAY_DEFAULT "SSD1306")
#set(PRUZEA_DISPLAY_DEFAULT "ILI9341")

# Storage options: SD, NONE
set(PRUZEA_STORAGE_DEFAULT "NONE")
#set(PRUZEA_STORAGE_DEFAULT "SD")

# Audio options: I2S, PWM, NONE
set(PRUZEA_AUDIO_DEFAULT "PWM")
#set(PRUZEA_AUDIO_DEFAULT "I2S")
#set(PRUZEA_AUDIO_DEFAULT "NONE")

# Input options: GPIO_BUTTONS, SNES 
set(PRUZEA_INPUT_DEFAULT "GPIO_BUTTONS")
#set(PRUZEA_INPUT_DEFAULT "SNES")

# Japanese font options: ON, OFF
set(PRUZEA_JAPANESE_FONT_DEFAULT OFF)
#set(PRUZEA_JAPANESE_FONT_DEFAULT ON)

# PSRAM options: ON, OFF
set(PRUZEA_PSRAM_DEFAULT OFF)
#set(PRUZEA_PSRAM_DEFAULT ON)

# Sample game/tool options: ON, OFF
set(PRUZEA_SAMPLES_DEFAULT ON)
#set(PRUZEA_SAMPLES_DEFAULT OFF)

# Optional developer-specific overrides. This file is ignored by Git.
set(PRUZEA_PICOTOOL_PATH_DEFAULT "")
include("${CMAKE_CURRENT_LIST_DIR}/CMakeLocalConfig.cmake" OPTIONAL)

###### ENVIRONMENT END #######



set(PRUZEA_TARGET "${PRUZEA_TARGET_DEFAULT}" CACHE STRING "RP2040 or RP2350")
set_property(CACHE PRUZEA_TARGET PROPERTY STRINGS RP2040 RP2350)

set(PRUZEA_DISPLAY "${PRUZEA_DISPLAY_DEFAULT}" CACHE STRING "SSD1306 or ILI9341")
set_property(CACHE PRUZEA_DISPLAY PROPERTY STRINGS SSD1306 ILI9341)

set(PRUZEA_STORAGE "${PRUZEA_STORAGE_DEFAULT}" CACHE STRING "SD or NONE")
set_property(CACHE PRUZEA_STORAGE PROPERTY STRINGS SD NONE)

set(PRUZEA_AUDIO "${PRUZEA_AUDIO_DEFAULT}" CACHE STRING "PWM, I2S, or NONE")
set_property(CACHE PRUZEA_AUDIO PROPERTY STRINGS I2S PWM NONE)

set(PRUZEA_INPUT "${PRUZEA_INPUT_DEFAULT}" CACHE STRING "GPIO_BUTTONS, SNES or PS")
set_property(CACHE PRUZEA_INPUT PROPERTY STRINGS GPIO_BUTTONS SNES PS)

option(PRUZEA_JAPANESE_FONT "Enable Japanese font" ${PRUZEA_JAPANESE_FONT_DEFAULT})
option(PRUZEA_PSRAM "Enable PSRAM" ${PRUZEA_PSRAM_DEFAULT})
option(PRUZEA_SAMPLES "Build sample games and tools" ${PRUZEA_SAMPLES_DEFAULT})

set(PRUZEA_PIN_CONFIG "${PRUZEA_PIN_CONFIG_DEFAULT}" CACHE FILEPATH "Pin config header")
set(PRUZEA_PICOTOOL_PATH "${PRUZEA_PICOTOOL_PATH_DEFAULT}" CACHE FILEPATH "Optional path to picotool executable used for automatic BOOTSEL reboot")

if (IS_ABSOLUTE "${PRUZEA_PIN_CONFIG}")
    file(RELATIVE_PATH PRUZEA_PIN_CONFIG_FROM_BINARY "${CMAKE_BINARY_DIR}" "${PRUZEA_PIN_CONFIG}")
    if (
        NOT PRUZEA_PIN_CONFIG_FROM_BINARY MATCHES "^\\.\\."
        AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PRUZEA_PIN_CONFIG_FROM_BINARY}"
    )
        set(PRUZEA_PIN_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/${PRUZEA_PIN_CONFIG_FROM_BINARY}")
    endif()
else()
    get_filename_component(
        PRUZEA_PIN_CONFIG
        "${PRUZEA_PIN_CONFIG}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()
set(PRUZEA_PIN_CONFIG "${PRUZEA_PIN_CONFIG}" CACHE FILEPATH "Pin config header" FORCE)

string(TOUPPER "${PRUZEA_TARGET}" PRUZEA_TARGET)
string(TOUPPER "${PRUZEA_DISPLAY}" PRUZEA_DISPLAY)
string(TOUPPER "${PRUZEA_STORAGE}" PRUZEA_STORAGE)
string(TOUPPER "${PRUZEA_AUDIO}" PRUZEA_AUDIO)
string(TOUPPER "${PRUZEA_INPUT}" PRUZEA_INPUT)

if (PRUZEA_TARGET STREQUAL "RP2040")
    set(PICO_BOARD pico CACHE STRING "" FORCE)
elseif (PRUZEA_TARGET STREQUAL "RP2350")
    set(PICO_BOARD pico2 CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "PRUZEA_TARGET must be RP2040 or RP2350")
endif()

foreach(option_name PRUZEA_DISPLAY PRUZEA_STORAGE PRUZEA_AUDIO PRUZEA_INPUT)
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

project(PRUZEA C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PRUZEA_PICOTOOL_PATH AND EXISTS "${PRUZEA_PICOTOOL_PATH}")
    add_executable(picotool IMPORTED GLOBAL)
    set_property(TARGET picotool PROPERTY IMPORTED_LOCATION "${PRUZEA_PICOTOOL_PATH}")
    set(picotool_FOUND 1)
endif()

pico_sdk_init()

add_subdirectory(lib/LovyanGFX)
if (PRUZEA_STORAGE STREQUAL "SD")
    add_subdirectory(lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src)
endif()

set(PRUZEA_DISPLAY_SSD1306 0)
set(PRUZEA_DISPLAY_ILI9341 0)
set(PRUZEA_STORAGE_SD 0)
set(PRUZEA_STORAGE_NONE 0)
set(PRUZEA_AUDIO_I2S 0)
set(PRUZEA_AUDIO_PWM 0)
set(PRUZEA_AUDIO_NONE 0)
set(PRUZEA_INPUT_SNES 0)
set(PRUZEA_INPUT_GPIO_BUTTONS 0)
set(PRUZEA_INPUT_PS 0)
set(PRUZEA_TARGET_RP2040 0)
set(PRUZEA_TARGET_RP2350 0)
set(PRUZEA_ENABLE_JAPANESE_FONT 0)
set(PRUZEA_ENABLE_PSRAM 0)

if (PRUZEA_DISPLAY STREQUAL "SSD1306")
    set(PRUZEA_DISPLAY_SSD1306 1)
elseif (PRUZEA_DISPLAY STREQUAL "ILI9341")
    set(PRUZEA_DISPLAY_ILI9341 1)
else()
    message(FATAL_ERROR "PRUZEA_DISPLAY must be SSD1306 or ILI9341")
endif()

if (PRUZEA_STORAGE STREQUAL "SD")
    set(PRUZEA_STORAGE_SD 1)
elseif (PRUZEA_STORAGE STREQUAL "NONE")
    set(PRUZEA_STORAGE_NONE 1)
else()
    message(FATAL_ERROR "PRUZEA_STORAGE must be SD or NONE")
endif()

if (PRUZEA_AUDIO STREQUAL "I2S")
    set(PRUZEA_AUDIO_I2S 1)
elseif (PRUZEA_AUDIO STREQUAL "PWM")
    set(PRUZEA_AUDIO_PWM 1)
elseif (PRUZEA_AUDIO STREQUAL "NONE")
    set(PRUZEA_AUDIO_NONE 1)
else()
    message(FATAL_ERROR "PRUZEA_AUDIO must be I2S, PWM, or NONE")
endif()

if (PRUZEA_INPUT STREQUAL "GPIO_BUTTONS")
    set(PRUZEA_INPUT_GPIO_BUTTONS 1)
elseif (PRUZEA_INPUT STREQUAL "SNES")
    set(PRUZEA_INPUT_SNES 1)
elseif (PRUZEA_INPUT STREQUAL "PS")
    set(PRUZEA_INPUT_PS 1)
else()
    message(FATAL_ERROR "PRUZEA_INPUT must be GPIO_BUTTONS, SNES or PS")
endif()

if (PRUZEA_TARGET STREQUAL "RP2040")
    set(PRUZEA_TARGET_RP2040 1)
else()
    set(PRUZEA_TARGET_RP2350 1)
endif()

if (PRUZEA_JAPANESE_FONT)
    set(PRUZEA_ENABLE_JAPANESE_FONT 1)
endif()

if (PRUZEA_PSRAM)
    set(PRUZEA_ENABLE_PSRAM 1)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/system/PRUZEAConfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/PRUZEAConfig.h
    @ONLY
)

add_subdirectory(system)

add_custom_target(deploy_pruzea_uf2
    COMMAND powershell
        -NoProfile
        -ExecutionPolicy Bypass
        -File "${CMAKE_SOURCE_DIR}/scripts/deploy-uf2.ps1"
        -Uf2Path "${CMAKE_BINARY_DIR}/system/pruzea.uf2"
        -PicotoolPath "${PRUZEA_PICOTOOL_PATH}"
    DEPENDS pruzea
    COMMENT "Deploying pruzea.uf2"
    VERBATIM
)
