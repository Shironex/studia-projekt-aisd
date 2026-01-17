@echo off
REM =============================================================================
REM BUILD.BAT - Skrypt kompilacji programu Huffmana
REM =============================================================================

echo =============================================
echo    KODOWANIE HUFFMANA - KOMPILACJA
echo =============================================
echo.

if "%1"=="clean" goto clean
if "%1"=="run" goto run

:build
REM Dodaj MSYS2 do PATH jesli istnieje
if exist "C:\msys64\mingw64\bin\g++.exe" (
    set "PATH=C:\msys64\mingw64\bin;%PATH%"
)

REM Probuj g++
where g++ >nul 2>nul
if %errorlevel%==0 goto use_gpp

REM Probuj cl.exe
where cl >nul 2>nul
if %errorlevel%==0 goto use_cl

goto no_compiler

:use_gpp
echo Uzywam kompilatora: g++
echo.
g++ -Wall -Wextra -std=c++11 -o huffman.exe huffman.cpp
if errorlevel 1 goto error
goto success

:use_cl
echo Uzywam kompilatora: cl.exe (Visual Studio)
echo.
cl /EHsc /Fe:huffman.exe huffman.cpp
if errorlevel 1 goto error
goto success

:no_compiler
echo BLAD: Nie znaleziono kompilatora C++!
echo.
echo Zainstaluj MinGW:
echo    winget install -e --id MSYS2.MSYS2
echo.
goto end

:success
echo.
echo =============================================
echo    KOMPILACJA UDANA!
echo =============================================
echo Uruchom: huffman.exe
goto end

:clean
if exist huffman.exe del /Q huffman.exe
if exist huffman.obj del /Q huffman.obj
echo Wyczyszczono!
goto end

:run
call :build
if exist huffman.exe huffman.exe
goto end

:error
echo.
echo KOMPILACJA NIE POWIODLA SIE!
goto end

:end
