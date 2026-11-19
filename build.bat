@echo off
setlocal
echo ============================================
echo   Scale Operator - build and install for FL Studio
echo ============================================
echo.

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] CMake was not found on your PATH.
    echo.
    echo Install Visual Studio 2022 Community (free) with the
    echo "Desktop development with C++" workload - that includes CMake.
    echo Download: https://visualstudio.microsoft.com/vs/community/
    echo.
    echo After installing, run this file again from a normal
    echo Command Prompt or by double-clicking it.
    pause
    exit /b 1
)

cd /d "%~dp0"

echo Step 1/2: Configuring the project...
echo (This downloads the JUCE framework the first time - needs internet,
echo  can take a few minutes.)
echo.
cmake -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo.
    echo [ERROR] Configure step failed - see the message above.
    echo Common fix: make sure Visual Studio 2022 is installed with the
    echo "Desktop development with C++" workload.
    pause
    exit /b 1
)

echo.
echo Step 2/2: Building the plugin...
echo.
cmake --build build --config Release
if errorlevel 1 (
    echo.
    echo [ERROR] Build step failed - see the message above.
    pause
    exit /b 1
)

echo.
echo ============================================
echo   Done! "Scale Operator.vst3" was copied to:
echo   C:\Program Files\Common Files\VST3
echo.
echo   Now open FL Studio, go to
echo   Options -^> Manage Plugins -^> Find Plugins
echo   and it'll show up under Generators/Instruments.
echo ============================================
pause
