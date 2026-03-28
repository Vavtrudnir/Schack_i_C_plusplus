@echo off
echo Setting up dependencies for Chess Application...
echo.

REM Set MSYS2 path
set MSYS_PATH=C:\msys64\mingw64\bin

REM Check if MSYS2 is installed
if not exist "%MSYS_PATH%" (
    echo Error: MSYS2 not found at %MSYS_PATH%
    echo Please install MSYS2 first: https://www.msys2.org/
    pause
    exit /b 1
)

echo Copying required DLL files...

REM Copy SFML DLLs
copy "%MSYS_PATH%\libsfml-graphics-3.dll" . >nul 2>&1
copy "%MSYS_PATH%\libsfml-window-3.dll" . >nul 2>&1
copy "%MSYS_PATH%\libsfml-system-3.dll" . >nul 2>&1

REM Copy runtime DLLs
copy "%MSYS_PATH%\libstdc++-6.dll" . >nul 2>&1
copy "%MSYS_PATH%\libgcc_s_seh-1.dll" . >nul 2>&1
copy "%MSYS_PATH%\libwinpthread-1.dll" . >nul 2>&1

REM Copy dependency DLLs
copy "%MSYS_PATH%\libfreetype-6.dll" . >nul 2>&1
copy "%MSYS_PATH%\libpng16-16.dll" . >nul 2>&1
copy "%MSYS_PATH%\zlib1.dll" . >nul 2>&1

echo.
echo Dependencies copied successfully!
echo.
echo You can now run the chess application using start_chess.bat
echo.
pause
