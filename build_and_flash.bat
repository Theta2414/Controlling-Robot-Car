@echo off
setlocal

echo ===============================================
echo Building STM32F103 Controlling-Robot-Car
echo ===============================================

REM Change to project directory
cd /d "%~dp0"

REM Clean build directory (using batch commands, not PowerShell)
if exist build (
    echo Cleaning build directory...
    rmdir /s /q build
)

REM Create build directory
mkdir build

REM Configure with CMake
echo.
echo [1/3] Configuring...
cmake -B build -G Ninja
if %ERRORLEVEL% neq 0 (
    echo Configuration failed!
    pause
    exit /b 1
)

REM Build
echo.
echo [2/3] Building...
cmake --build build
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

REM Summary
echo.
echo [3/3] Build Summary
echo ===============================================
if exist build\*.hex (
    dir build\*.hex | find ".hex"
) else (
    echo No .hex file generated!
)

if exist build\*.bin (
    dir build\*.bin | find ".bin"
) else (
    echo No .bin file generated!
)

echo ===============================================
echo Build completed successfully!
echo.

STM32_Programmer_CLI -c port=SWD sn=53FF6D066678485329492487 -w build\UART_test.elf -v -rst
pause
endlocal
