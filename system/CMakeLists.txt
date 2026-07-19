file(GLOB PLAMIO_GAME_DIRS CONFIGURE_DEPENDS
    LIST_DIRECTORIES true
    "${CMAKE_SOURCE_DIR}/games/*"
)

if (PLAMIO_SAMPLES)
    file(GLOB PLAMIO_SAMPLE_DIRS CONFIGURE_DEPENDS
        LIST_DIRECTORIES true
        "${CMAKE_SOURCE_DIR}/samples/*"
        "${CMAKE_SOURCE_DIR}/samples/tools/*"
    )
    list(APPEND PLAMIO_GAME_DIRS ${PLAMIO_SAMPLE_DIRS})
endif()

list(SORT PLAMIO_GAME_DIRS)

set(PLAMIO_GAME_SOURCES)
set(PLAMIO_GAME_INCLUDE_DIRS)
set(PLAMIO_GENERATED_GAME_INCLUDES)
set(PLAMIO_GENERATED_GAME_INSTANCES)
set(PLAMIO_GENERATED_GAME_REGISTRATIONS)

foreach(PLAMIO_GAME_DIR IN LISTS PLAMIO_GAME_DIRS)
    if (NOT IS_DIRECTORY "${PLAMIO_GAME_DIR}")
        continue()
    endif()

    get_filename_component(PLAMIO_GAME_NAME "${PLAMIO_GAME_DIR}" NAME)
    if (PLAMIO_GAME_NAME MATCHES "^_")
        continue()
    endif()

    set(PLAMIO_GAME_HEADER "${PLAMIO_GAME_DIR}/${PLAMIO_GAME_NAME}.h")
    set(PLAMIO_GAME_SOURCE "${PLAMIO_GAME_DIR}/${PLAMIO_GAME_NAME}.cpp")
    if (NOT EXISTS "${PLAMIO_GAME_HEADER}" OR NOT EXISTS "${PLAMIO_GAME_SOURCE}")
        continue()
    endif()

    list(APPEND PLAMIO_GAME_SOURCES "${PLAMIO_GAME_SOURCE}")
    list(APPEND PLAMIO_GAME_INCLUDE_DIRS "${PLAMIO_GAME_DIR}")
    string(APPEND PLAMIO_GENERATED_GAME_INCLUDES "#include \"${PLAMIO_GAME_NAME}.h\"\n")
    string(APPEND PLAMIO_GENERATED_GAME_INSTANCES "static ${PLAMIO_GAME_NAME} plamioGame_${PLAMIO_GAME_NAME};\n")
    string(APPEND PLAMIO_GENERATED_GAME_REGISTRATIONS "    catalog.addGame(&plamioGame_${PLAMIO_GAME_NAME}, graphics);\n")
endforeach()

set(PLAMIO_GENERATED_GAMES_H "${CMAKE_BINARY_DIR}/generated/PLAMIOGeneratedGames.h")
set(PLAMIO_GENERATED_GAMES_CPP "${CMAKE_BINARY_DIR}/generated/PLAMIOGeneratedGames.cpp")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")
file(WRITE "${PLAMIO_GENERATED_GAMES_H}"
"#pragma once

namespace PLAMIO {

class GameCatalog;
class GraphicsBase;

void registerGeneratedGames(GameCatalog& catalog, GraphicsBase& graphics);

} // namespace PLAMIO
")
file(WRITE "${PLAMIO_GENERATED_GAMES_CPP}"
"#include \"PLAMIOGeneratedGames.h\"
#include \"GameCatalog.h\"
#include \"GraphicsBase.h\"
${PLAMIO_GENERATED_GAME_INCLUDES}
namespace PLAMIO {

${PLAMIO_GENERATED_GAME_INSTANCES}
void registerGeneratedGames(GameCatalog& catalog, GraphicsBase& graphics)
{
${PLAMIO_GENERATED_GAME_REGISTRATIONS}}

} // namespace PLAMIO
")

add_executable(plamio
    platform/pico/main.cpp
    platform/pico/Platform.cpp
    hardware/InputBase.cpp
    hardware/GraphicsBase.cpp
    hardware/AudioBase.cpp
    hardware/StorageBase.cpp
    ui/SystemUI.cpp
    ui/CoreRing.cpp
    core/System.cpp
    core/GameCatalog.cpp
    util/Tween.cpp
    util/Math.cpp
    util/SaveData.cpp
    util/Collision.cpp
    util/ScreenShot.cpp
    ${PLAMIO_GENERATED_GAMES_CPP}
    ${PLAMIO_GAME_SOURCES}
)

target_include_directories(plamio PRIVATE
    ${CMAKE_SOURCE_DIR}/sdk
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/core
    ${CMAKE_CURRENT_LIST_DIR}/ui
    ${CMAKE_CURRENT_LIST_DIR}/util
    ${CMAKE_CURRENT_LIST_DIR}/hardware
    ${CMAKE_CURRENT_LIST_DIR}/platform/pico
    ${CMAKE_CURRENT_LIST_DIR}/platform/pico/boards
    ${PLAMIO_GAME_INCLUDE_DIRS}
)

if (PLAMIO_DISPLAY STREQUAL "ILI9341")
    target_sources(plamio PRIVATE
        platform/pico/GraphicsILI9341.cpp
        ui/SystemUI320x240.cpp
        ui/SystemUI128x64Mono.cpp
    )
elseif (PLAMIO_DISPLAY STREQUAL "SSD1306")
    target_sources(plamio PRIVATE
        platform/pico/GraphicsSSD1306.cpp
        ui/SystemUI128x64Mono.cpp
    )
endif()

if (PLAMIO_STORAGE STREQUAL "SD")
    target_sources(plamio PRIVATE platform/pico/StorageSD.cpp)
elseif (PLAMIO_STORAGE STREQUAL "NONE")
    target_sources(plamio PRIVATE hardware/StorageStub.cpp)
endif()

if (PLAMIO_AUDIO STREQUAL "I2S")
    target_sources(plamio PRIVATE platform/pico/AudioI2S.cpp)
elseif (PLAMIO_AUDIO STREQUAL "PWM")
    target_sources(plamio PRIVATE platform/pico/AudioPWM.cpp)
elseif (PLAMIO_AUDIO STREQUAL "NONE")
    target_sources(plamio PRIVATE hardware/AudioStub.cpp)
endif()

if (PLAMIO_INPUT STREQUAL "SNES")
    target_sources(plamio PRIVATE
        platform/pico/GpioButtons.cpp
        platform/pico/InputSnes.cpp
    )
elseif (PLAMIO_INPUT STREQUAL "GPIO_BUTTONS")
    target_sources(plamio PRIVATE
        platform/pico/GpioButtons.cpp
        platform/pico/InputGpioButtons.cpp
    )
elseif (PLAMIO_INPUT STREQUAL "PS2")
    target_sources(plamio PRIVATE
        platform/pico/GpioButtons.cpp
        platform/pico/InputPS2.cpp
    )
endif()

target_link_libraries(plamio
    pico_stdlib
    pico_multicore
    hardware_gpio
    hardware_adc
    hardware_spi
    hardware_i2c
    hardware_pwm
    hardware_dma
    hardware_pio
    LovyanGFX
)

if (PLAMIO_AUDIO STREQUAL "I2S")
    target_link_libraries(plamio pico_audio_i2s)
endif()

if (PLAMIO_STORAGE STREQUAL "SD")
    target_link_libraries(plamio no-OS-FatFS-SD-SDIO-SPI-RPi-Pico)
endif()

pico_enable_stdio_usb(plamio 1)
pico_enable_stdio_uart(plamio 0)

pico_add_extra_outputs(plamio)

add_custom_command(TARGET plamio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:plamio>/plamio.uf2
        ${CMAKE_BINARY_DIR}/plamio.uf2
    COMMENT "Copying plamio.uf2 to build directory"
)
