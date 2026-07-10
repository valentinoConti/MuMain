@echo off
REM Wrapper: set up MSVC x64 dev environment + tool paths, then run passed command.
REM (Same as build-env.cmd but targets the 64-bit toolchain.)
cd /d "%~dp0"
set "PATH=C:\Program Files\CMake\bin;C:\Program Files\dotnet;C:\Users\PC\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Program Files (x86)\Microsoft Visual Studio\Installer;%PATH%"
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
if errorlevel 1 (echo VCVARS FAILED & exit /b 1)
%*
