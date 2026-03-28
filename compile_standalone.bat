@echo off
echo Compiling standalone chess application...
echo.

set GCC_PATH=C:\msys64\mingw64\bin
set INCLUDE_PATH=C:\msys64\mingw64\include
set LIB_PATH=C:\msys64\mingw64\lib

echo Using GCC from: %GCC_PATH%
echo.

%GCC_PATH%\g++.exe -std=c++17 ^
    -I"%INCLUDE_PATH%" ^
    -L"%LIB_PATH%" ^
    -o schack_standalone.exe ^
    schack_med_bilder.cpp ^
    -lsfml-graphics ^
    -lsfml-window ^
    -lsfml-system ^
    -lwinmm ^
    -lgdi32 ^
    -lopengl32 ^
    -static-libgcc ^
    -static-libstdc++

if errorlevel 1 (
    echo.
    echo Compilation failed!
    pause
    exit /b 1
)

echo.
echo Compilation successful!
echo Created: schack_standalone.exe
echo.
pause
