@echo off
echo Starting Chess Application...
echo.

REM Add MSYS2 to PATH so all DLLs are found automatically
set PATH=C:\msys64\mingw64\bin;%PATH%

REM Check if executable exists
if not exist "schack_standalone.exe" (
    echo Error: schack_standalone.exe not found!
    echo Please compile the application first.
    pause
    exit /b 1
)

echo Launching chess game...
echo.
echo Controls:
echo - Click a piece to select it
echo - Click destination to move
echo - White (human) vs Black (computer)
echo.

REM Start the application
schack_standalone.exe

if errorlevel 1 (
    echo.
    echo Application exited with error.
    pause
)
