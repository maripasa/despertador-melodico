cmake_minimum_required(VERSION 3.13)

# Set C and C++ standards.
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# Set the board type (change this if you are using a different board).
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in the Raspberry Pi Pico SDK (must be before project).
include(pico_sdk_import.cmake)

# Define the project.
project(KeyTrainerProject C CXX ASM)

# Initialise the Raspberry Pi Pico SDK.
pico_sdk_init()

# Add the executable.
# Adjust the file list below if you add or remove source files.
add_executable(key_trainer
    key_trainer.c
    ssd1306_i2c.c
    play_audio.c
)

# Set the program name and version.
pico_set_program_name(key_trainer "KeyTrainer")
pico_set_program_version(key_trainer "0.1")

# Enable/disable UART and USB stdio.
pico_enable_stdio_uart(key_trainer 0)
pico_enable_stdio_usb(key_trainer 1)

# Link required libraries.
# Note: hardware_pwm is now added so that hardware/pwm.h can be found.
target_link_libraries(key_trainer
    pico_stdlib
    hardware_i2c
    hardware_adc
    hardware_pwm
)

# Include the current directory and one level up (if needed for headers).
target_include_directories(key_trainer PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/..
)

# Add extra outputs (UF2, etc.) supported by the Pico SDK.
pico_add_extra_outputs(key_trainer)
