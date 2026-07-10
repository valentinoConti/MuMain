@echo off
REM ============================================================
REM  Build the 32-bit MuMain client (windows-x86 preset).
REM  - First run auto-configures the x86 build tree.
REM  - If Release\Data is missing, forces a full asset re-copy.
REM  - Recompiles only what changed.
REM  Output: out\build\windows-x86\src\Release\Main.exe
REM  NOTE: for the ORIGINAL-textures client, run "Build Client original textures.cmd"
REM        (this script alone leaves the HD/4x textures in place, which crash x86).
REM ============================================================
if not exist "%~dp0out\build\windows-x86\CMakeCache.txt" (
  echo First run - configuring x86 build tree...
  call "%~dp0build-env-x86.cmd" cmake --preset windows-x86
)
if not exist "%~dp0out\build\windows-x86\src\Release\Data" (
  echo Release\Data missing - forcing a full asset re-copy...
  del /q "%~dp0out\build\windows-x86\src\.assets_copied_Release.stamp" 2>nul
)
call "%~dp0build-env-x86.cmd" cmake --build --preset windows-x86-release
echo.
echo ============================================================
echo  Done. Client: out\build\windows-x86\src\Release\Main.exe
echo ============================================================
