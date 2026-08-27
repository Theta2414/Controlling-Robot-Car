cmake_minimum_required(VERSION 3.20)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(Rx_car C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# =========================================================
# MCU FLAGS
# STM32F103C8T6 = Cortex-M3
# =========================================================

set(MCU_FLAGS
    -mcpu=cortex-m3
    -mthumb
)

# =========================================================
# SOURCE FILES
# =========================================================

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS
    Core/Src/*.c
)

file(GLOB HAL_SOURCES
    Drivers/STM32F1xx_HAL_Driver/Src/*.c
)

file(GLOB STARTUP_SOURCE
    Core/Startup/*.s
)

add_executable(${PROJECT_NAME}.elf
    ${CORE_SOURCES}
    ${HAL_SOURCES}
    ${STARTUP_SOURCE}
)

# =========================================================
# INCLUDE DIRECTORIES
# =========================================================

target_include_directories(${PROJECT_NAME}.elf PRIVATE
    Core/Inc
    Drivers/STM32F1xx_HAL_Driver/Inc
    Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F1xx/Include
    Drivers/CMSIS/Include
)

# =========================================================
# DEFINES
# =========================================================

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    USE_HAL_DRIVER
    STM32F103xB
)

# =========================================================
# COMPILER OPTIONS
# =========================================================

target_compile_options(${PROJECT_NAME}.elf PRIVATE
    ${MCU_FLAGS}

    -Wall
    -Wextra

    -ffunction-sections
    -fdata-sections

    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g3>
)

# =========================================================
# LINKER
# =========================================================

target_link_options(${PROJECT_NAME}.elf PRIVATE
    ${MCU_FLAGS}

    -T${CMAKE_SOURCE_DIR}/STM32F103C8TX_FLASH.ld

    --specs=nano.specs
    --specs=nosys.specs

    -Wl,--gc-sections
    -Wl,-Map=${PROJECT_NAME}.map
)

target_link_libraries(${PROJECT_NAME}.elf
    m
)