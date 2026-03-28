@echo off
echo Kompilerar schack_med_bilder.cpp...

REM Försök kompilera med g++ om tillgängligt
g++ -o schack_new.exe schack_med_bilder.cpp -lsfml-graphics -lsfml-window -lsfml-system -std=c++17 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Kompilering lyckades med g++!
    goto :end
)

REM Försök med MinGW om tillgängligt
mingw32-g++ -o schack_new.exe schack_med_bilder.cpp -lsfml-graphics -lsfml-window -lsfml-system -std=c++17 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Kompilering lyckades med MinGW!
    goto :end
)

echo Ingen kompilator hittades. Försök att:
echo 1. Installera MinGW eller Visual Studio
echo 2. Lägg till kompilatorn i PATH
echo 3. Eller använd din vanliga utvecklingsmiljö för att kompilera schack_med_bilder.cpp
echo.
echo Krävs bibliotek: SFML (graphics, window, system)
echo Krävs headers: SFML, Windows.h, C++17 stöd

:end
echo Klar.
