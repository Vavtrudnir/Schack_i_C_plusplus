@echo off
echo ================================================
echo   SKAPAR FRISTAENDE SCHACKPROGRAM
echo ================================================
echo.

REM Skapa release-mapp
if exist "chess_release" rmdir /s /q chess_release
mkdir chess_release

echo Kopierar programfiler...
copy schack_standalone.exe chess_release\Schack.exe
copy stockfish.exe chess_release\

echo Kopierar bilder...
xcopy /E /I /Y bilder chess_release\bilder

echo Kopierar SFML DLL-filer...
copy C:\msys64\mingw64\bin\libsfml-graphics-3.dll chess_release\
copy C:\msys64\mingw64\bin\libsfml-window-3.dll chess_release\
copy C:\msys64\mingw64\bin\libsfml-system-3.dll chess_release\

echo Kopierar runtime DLL-filer...
copy C:\msys64\mingw64\bin\libstdc++-6.dll chess_release\
copy C:\msys64\mingw64\bin\libgcc_s_seh-1.dll chess_release\
copy C:\msys64\mingw64\bin\libwinpthread-1.dll chess_release\

echo Kopierar beroende DLL-filer...
copy C:\msys64\mingw64\bin\libfreetype-6.dll chess_release\
copy C:\msys64\mingw64\bin\libpng16-16.dll chess_release\
copy C:\msys64\mingw64\bin\zlib1.dll chess_release\
copy C:\msys64\mingw64\bin\libbrotlidec.dll chess_release\
copy C:\msys64\mingw64\bin\libbrotlicommon.dll chess_release\
copy C:\msys64\mingw64\bin\libbz2-1.dll chess_release\
copy C:\msys64\mingw64\bin\libharfbuzz-0.dll chess_release\
copy C:\msys64\mingw64\bin\libglib-2.0-0.dll chess_release\
copy C:\msys64\mingw64\bin\libgraphite2.dll chess_release\
copy C:\msys64\mingw64\bin\libintl-8.dll chess_release\
copy C:\msys64\mingw64\bin\libiconv-2.dll chess_release\
copy C:\msys64\mingw64\bin\libpcre2-8-0.dll chess_release\

echo.
echo Skapar ZIP-arkiv...
powershell Compress-Archive -Path chess_release\* -DestinationPath Schackprogram_Fristaende.zip -Force

echo.
echo ================================================
echo   KLART!
echo ================================================
echo.
echo Fristaende program skapat i: chess_release\
echo ZIP-arkiv skapat: Schackprogram_Fristaende.zip
echo.
echo Du kan nu:
echo 1. Kopiera chess_release-mappen till vilken dator som helst
echo 2. Eller skicka Schackprogram_Fristaende.zip
echo.
echo Starta programmet med: chess_release\Starta_Schack.bat
echo.
pause
