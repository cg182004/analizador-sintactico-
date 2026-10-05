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
    echo No se encontro GCC. Instala MinGW-w64 y configura el PATH.
    exit /b 1
)
if not exist "dist" mkdir "dist"
gcc tests\pruebas.c -o dist\pruebas.exe -std=c11 -Wall -Wextra -O2 -static -luser32 -lgdi32
if errorlevel 1 exit /b 1
dist\pruebas.exe
exit /b %errorlevel%
