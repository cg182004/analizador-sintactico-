@echo off
setlocal
cd /d "%~dp0"

where gcc >nul 2>&1
if errorlevel 1 (
    if exist "C:\msys64\ucrt64\bin\gcc.exe" (
        set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    )
)

where gcc >nul 2>&1
if errorlevel 1 (
    echo No se encontro GCC.
    echo Instala MinGW-w64 y agrega su carpeta bin al PATH.
    pause
    exit /b 1
)

if not exist "dist" mkdir "dist"

gcc main.c -o "dist\AnalizadorSintactico.exe" -std=c11 -Wall -Wextra -O2 -mwindows -static -luser32 -lgdi32

if errorlevel 1 (
    echo.
    echo La compilacion fallo. Revisa los errores anteriores.
    pause
    exit /b 1
)

echo.
echo Ejecutable creado: dist\AnalizadorSintactico.exe
start "" "dist\AnalizadorSintactico.exe"
endlocal
